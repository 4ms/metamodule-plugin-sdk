// Test plugin for deadlocks and arena corruption in the firmware's newlib locks (firmware/system/lock.cc). See
// README.md.
//
// One module that allocates from the plugin arena in three contexts at once, each with its own knob for how many
// allocations it does per call:
//
//   Text display:    get_display_text(), which the firmware calls in the ReadPatchGuiElements interrupt on core 1
//   Graphic display: draw(), on the GUI thread (Core1Main)
//   AsyncThread:     on the core the module is assigned to (shown on the text display)
//
// A fourth knob sets how much DSP work process() does, to give the audio interrupts realistic, varying lengths.
//
// Every context checks that its strings still hold what it wrote, so arena corruption (two contexts handed the same
// block) shows up on the text display's error count, usually before it crashes the firmware.
//
// If the firmware freezes, the text display's counters, the graphic display's moving bar and the heartbeat light
// (blinked by process()) all stop.

#include <rack.hpp>

#include "threads/async_thread.hh"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstdio>
#include <span>
#include <string>

using namespace rack;

Plugin *pluginInstance;

namespace
{

int current_core() {
	unsigned mpidr;
	asm volatile("mrc p15, 0, %0, c0, c0, 5" : "=r"(mpidr)); // MPIDR
	return mpidr & 0b1;
}

std::atomic<unsigned> total_errors{0}; // all modules

// Strings that each allocate (they're longer than the small string buffer), and what was written to each one
template<size_t N>
struct Slots {
	std::array<std::string, N> strings;
	std::array<char, N> fill{};
	std::array<uint8_t, N> length{};

	// Does `count` allocations and as many frees: each new string replaces the one in a slot, which frees the old
	// one. Before replacing a string, checks it still holds what was written to it. Returns the number of strings
	// that didn't.
	unsigned churn(unsigned count) {
		unsigned errors = 0;
		for (unsigned i = 0; i < count; i++) {
			auto slot = i % N;
			if (!intact(slot))
				errors++;

			length[slot] = 32 + (i & 31);
			fill[slot] = char('a' + (i & 15));
			strings[slot] = std::string(length[slot], fill[slot]);
		}
		return errors;
	}

	bool intact(size_t slot) const {
		auto &s = strings[slot];
		if (s.size() != length[slot])
			return false;
		return std::all_of(s.begin(), s.end(), [c = fill[slot]](char x) { return x == c; });
	}
};

struct LockTest : Module {
	enum ParamIds { TEXT_ALLOCS_PARAM, DRAW_ALLOCS_PARAM, ASYNC_ALLOCS_PARAM, DSP_LOAD_PARAM, NUM_PARAMS };
	enum InputIds { SIGNAL_INPUT, NUM_INPUTS };
	enum OutputIds { SIGNAL_OUTPUT, NUM_OUTPUTS };
	// A text display needs a light id of its own
	enum LightIds { HEARTBEAT_LIGHT, TEXT_DISPLAY, NUM_LIGHTS };

	LockTest() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(TEXT_ALLOCS_PARAM, 0, 400, 0, "Text display allocations per read");
		configParam(DRAW_ALLOCS_PARAM, 0, 2000, 0, "Graphic display allocations per frame");
		configParam(ASYNC_ALLOCS_PARAM, 0, 4000, 0, "AsyncThread allocations per pass");
		configParam(DSP_LOAD_PARAM, 0, 800, 0, "DSP work per sample (average loop iterations)");
		configInput(SIGNAL_INPUT, "Mixed into the DSP work");
		configOutput(SIGNAL_OUTPUT, "Result of the DSP work (quiet)");
		async.start();
	}

	~LockTest() override {
		async.stop();
	}

	void process(const ProcessArgs &args) override {
		auto core = current_core();
		if (audio_core.load(std::memory_order_relaxed) != core)
			audio_core.store(core, std::memory_order_relaxed);

		outputs[SIGNAL_OUTPUT].setVoltage(dsp_work(inputs[SIGNAL_INPUT].getVoltage()));

		blink_time += args.sampleTime;
		if (blink_time >= 0.25f) {
			blink_time -= 0.25f;
			heartbeat = !heartbeat;
			lights[HEARTBEAT_LIGHT].setBrightness(heartbeat ? 1.f : 0.f);
		}
	}

	// Stands in for a module's DSP: reads and writes a 16kB table, in a dependent chain the compiler can't skip.
	// The number of iterations varies randomly from half to one and a half times the knob's value, so the audio
	// interrupts end at different times relative to the GUI and the AsyncThreads.
	float dsp_work(float in) {
		auto base = allocs(DSP_LOAD_PARAM);
		if (base == 0)
			return 0.f;

		auto iterations = base / 2 + next_random() % (base + 1);
		for (unsigned i = 0; i < iterations; i++) {
			auto r = next_random();
			acc = acc * 0.999f + table[r % table.size()] * 0.001f;
			table[(r >> 16) % table.size()] = acc + in * 0.01f;
		}
		return acc * 0.1f;
	}

	uint32_t next_random() {
		// xorshift32
		rng ^= rng << 13;
		rng ^= rng >> 17;
		rng ^= rng << 5;
		return rng;
	}

	// Called in the ReadPatchGuiElements interrupt on core 1, while this display is on screen
	size_t get_display_text(int display_id, std::span<char> text) override {
		if (display_id != TEXT_DISPLAY || text.size() < 2)
			return 0;

		// No name, so it isn't printed: printf here would take the stdio lock, which changes what's being tested
		count_errors(text_slots.churn(allocs(TEXT_ALLOCS_PARAM)), nullptr);
		auto reads = text_reads.fetch_add(1) + 1;

		// snprintf into a buffer, without %f, doesn't allocate or take a lock
		int len = std::snprintf(text.data(),
								text.size(),
								"Text %u\nDraw %u\nAsync %u\nCore: audio %d async %d\nErrors %u/%u",
								reads,
								draw_frames.load(),
								async_passes.load(),
								audio_core.load(),
								async_core.load(),
								errors.load(),
								total_errors.load());
		// The firmware terminates the text at the returned length, so leave room for that
		return len < 0 ? 0 : std::min<size_t>(len, text.size() - 1);
	}

	void async_pass() {
		auto core = current_core();
		if (async_core.exchange(core) != core)
			printf("[LockTest] AsyncThread is on core %d\n", core);

		count_errors(async_slots.churn(allocs(ASYNC_ALLOCS_PARAM)), "AsyncThread");
		async_passes.fetch_add(1);
	}

	// Prints the errors if `where` is given
	void count_errors(unsigned n, const char *where) {
		if (n == 0)
			return;
		errors.fetch_add(n);
		total_errors.fetch_add(n);
		if (where)
			printf("[LockTest] %u corrupted strings in the %s (core %d)\n", n, where, current_core());
	}

	unsigned allocs(int param_id) {
		return unsigned(std::max(params[param_id].getValue(), 0.f));
	}

	std::atomic<unsigned> text_reads{0};
	std::atomic<unsigned> draw_frames{0};
	std::atomic<unsigned> async_passes{0};
	std::atomic<unsigned> errors{0};
	std::atomic<int> async_core{-1};
	std::atomic<int> audio_core{-1};

	float blink_time = 0;
	bool heartbeat = false;

	float acc = 0;
	uint32_t rng = 0x12345678;
	std::array<float, 4096> table{};

	// One set of slots per context, so no two contexts touch the same strings
	Slots<8> text_slots;
	Slots<8> async_slots;

	MetaModule::AsyncThread async{this, [this]() { async_pass(); }};
};

// Graphic display: draw() runs on the GUI thread (Core1Main)
struct ChurnDisplay : Widget {
	LockTest *module = nullptr;
	Slots<8> slots;
	unsigned frame = 0;

	void draw(const DrawArgs &args) override {
		frame++;
		if (module) {
			module->count_errors(slots.churn(module->allocs(LockTest::DRAW_ALLOCS_PARAM)), "graphic display");
			module->draw_frames.fetch_add(1);
		}

		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGB(0x10, 0x18, 0x20));
		nvgFill(args.vg);

		// A moving bar, to show the GUI is still drawing
		float x = (frame % 60) / 60.f * (box.size.x - 4);
		nvgBeginPath(args.vg);
		nvgRect(args.vg, x, 0, 4, box.size.y);
		nvgFillColor(args.vg, nvgRGB(0x40, 0xc0, 0xff));
		nvgFill(args.vg);
	}
};

struct LockTestWidget : ModuleWidget {
	LockTestWidget(LockTest *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panel.svg")));

		auto text = createWidget<MetaModule::VCVTextDisplay>(mm2px(Vec(4, 8)));
		text->box.size = mm2px(Vec(40, 34));
		text->firstLightId = LockTest::TEXT_DISPLAY;
		text->font = "Default_10";
		text->color = Colors565::Yellow;
		addChild(text);

		auto graphic = createWidget<ChurnDisplay>(mm2px(Vec(48, 8)));
		graphic->box.size = mm2px(Vec(39, 34));
		graphic->module = module;
		addChild(graphic);

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 58)), module, LockTest::TEXT_ALLOCS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45, 58)), module, LockTest::DRAW_ALLOCS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(75, 58)), module, LockTest::ASYNC_ALLOCS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 84)), module, LockTest::DSP_LOAD_PARAM));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(45, 84)), module, LockTest::HEARTBEAT_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(66, 84)), module, LockTest::SIGNAL_INPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(80, 84)), module, LockTest::SIGNAL_OUTPUT));
	}
};

} // namespace

void init(Plugin *p) {
	pluginInstance = p;
	p->addModel(createModel<LockTest, LockTestWidget>("LockTest"));
}
