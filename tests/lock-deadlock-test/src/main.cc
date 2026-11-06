// Test plugin for deadlocks in the firmware's newlib locks (firmware/system/lock.cc). See README.md.
//
// One module that allocates from the plugin arena in three contexts at once, each with its own knob for how many
// allocations it does per call:
//
//   Text display:    get_display_text(), which the firmware calls in the ReadPatchGuiElements interrupt on core 1
//   Graphic display: draw(), on the GUI thread (Core1Main)
//   AsyncThread:     on the core the module is assigned to (shown on the text display)
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

// Does `count` allocations and as many frees: each new string replaces the one in a slot, which frees the old one.
// The strings are longer than the small string buffer, so every one of them allocates. (Storing them in the slots
// also keeps the compiler from leaving out the allocations.)
template<size_t N>
void churn(std::array<std::string, N> &slots, unsigned count) {
	for (unsigned i = 0; i < count; i++)
		slots[i % N] = std::string(32 + (i & 31), char('a' + (i & 15)));
}

struct LockTest : Module {
	enum ParamIds { TEXT_ALLOCS_PARAM, DRAW_ALLOCS_PARAM, ASYNC_ALLOCS_PARAM, NUM_PARAMS };
	// A text display needs a light id of its own
	enum LightIds { HEARTBEAT_LIGHT, TEXT_DISPLAY, NUM_LIGHTS };

	LockTest() {
		config(NUM_PARAMS, 0, 0, NUM_LIGHTS);
		configParam(TEXT_ALLOCS_PARAM, 0, 400, 0, "Text display allocations per read");
		configParam(DRAW_ALLOCS_PARAM, 0, 2000, 0, "Graphic display allocations per frame");
		configParam(ASYNC_ALLOCS_PARAM, 0, 4000, 0, "AsyncThread allocations per pass");
		async.start();
	}

	~LockTest() override {
		async.stop();
	}

	void process(const ProcessArgs &args) override {
		blink_time += args.sampleTime;
		if (blink_time >= 0.25f) {
			blink_time -= 0.25f;
			heartbeat = !heartbeat;
			lights[HEARTBEAT_LIGHT].setBrightness(heartbeat ? 1.f : 0.f);
		}
	}

	// Called in the ReadPatchGuiElements interrupt on core 1, while this display is on screen
	size_t get_display_text(int display_id, std::span<char> text) override {
		if (display_id != TEXT_DISPLAY || text.size() < 2)
			return 0;

		churn(text_slots, allocs(TEXT_ALLOCS_PARAM));
		auto reads = text_reads.fetch_add(1) + 1;

		// snprintf into a buffer, without %f, doesn't allocate or take a lock
		int len = std::snprintf(text.data(),
								text.size(),
								"Text %u\nDraw %u\nAsync %u\nAsync core %d",
								reads,
								draw_frames.load(),
								async_passes.load(),
								async_core.load());
		// The firmware terminates the text at the returned length, so leave room for that
		return len < 0 ? 0 : std::min<size_t>(len, text.size() - 1);
	}

	void async_pass() {
		auto core = current_core();
		if (async_core.exchange(core) != core)
			printf("[LockTest] AsyncThread is on core %d\n", core);

		churn(async_slots, allocs(ASYNC_ALLOCS_PARAM));
		async_passes.fetch_add(1);
	}

	unsigned allocs(int param_id) {
		return unsigned(std::max(params[param_id].getValue(), 0.f));
	}

	std::atomic<unsigned> text_reads{0};
	std::atomic<unsigned> draw_frames{0};
	std::atomic<unsigned> async_passes{0};
	std::atomic<int> async_core{-1};

	float blink_time = 0;
	bool heartbeat = false;

	// One set of slots per context, so no two contexts touch the same strings
	std::array<std::string, 8> text_slots;
	std::array<std::string, 8> async_slots;

	MetaModule::AsyncThread async{this, [this]() { async_pass(); }};
};

// Graphic display: draw() runs on the GUI thread (Core1Main)
struct ChurnDisplay : Widget {
	LockTest *module = nullptr;
	std::array<std::string, 8> slots;
	unsigned frame = 0;

	void draw(const DrawArgs &args) override {
		frame++;
		if (module) {
			churn(slots, module->allocs(LockTest::DRAW_ALLOCS_PARAM));
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15, 62)), module, LockTest::TEXT_ALLOCS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(45, 62)), module, LockTest::DRAW_ALLOCS_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(75, 62)), module, LockTest::ASYNC_ALLOCS_PARAM));

		addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(45, 90)), module, LockTest::HEARTBEAT_LIGHT));
	}
};

} // namespace

void init(Plugin *p) {
	pluginInstance = p;
	p->addModel(createModel<LockTest, LockTestWidget>("LockTest"));
}
