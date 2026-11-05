// Repro for plugin-arena heap aborts (data abort in mm_tlsf_free /
// mm_tlsf_memalign), cut down from Robot Boy's Weaver preset handling.
//
// Each module runs an AsyncThread that does what Weaver's file service did
// before it was made lean. The variants switch the pieces on and off:
//
//   Churn: every pass (2 kHz), copy a fixed-size name into a std::string,
//          compare it with the listed entries and copy the match back out.
//   Probes: on each step, opendir six folders that don't exist, using the
//          doubled-volume form ("sdc:/sdc:/...") Weaver uses to get around
//          opendir() ignoring the volume.
//   List:  on each step, list "sdc:/sdc:/" (the card root) into a
//          std::vector<std::string>.
//   Read:  on each step, fopen and json_loads every .json file at the root,
//          keeping each as an entry with two std::strings.
//   Display: two graphic displays that redraw text (built in std::strings)
//          and a moving line every frame, as Weaver's displays do.
//
// process() requests a step every 100 ms. No params or jacks. Progress is
// printed to the console every 100 steps.

#include <rack.hpp>

#include "filesystem/dirent.h"
#include "threads/async_thread.hh"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

using namespace rack;

Plugin *pluginInstance;

namespace
{

enum : unsigned { kChurn = 1, kProbes = 2, kList = 4, kRead = 8, kDisplay = 16 };

constexpr unsigned kStepSamples = 4800; // 100 ms at 48 kHz

struct Entry {
	std::string name;
	std::string file;
};

template<unsigned Flags>
struct ChurnModule : Module {
	ChurnModule() {
		config(0, 0, 0, 0);
		std::snprintf(current, sizeof current, "%s", "no entry yet, waiting for the first step");
		printf("[churn-test] module created, flags %u (churn %d probes %d list %d read %d display %d)\n",
			   Flags,
			   (Flags & kChurn) != 0,
			   (Flags & kProbes) != 0,
			   (Flags & kList) != 0,
			   (Flags & kRead) != 0,
			   (Flags & kDisplay) != 0);
		if (Flags & (kChurn | kProbes | kList | kRead))
			thread.start();
	}

	~ChurnModule() override {
		thread.stop();
	}

	void process(const ProcessArgs &) override {
		if (++samples >= kStepSamples) {
			samples = 0;
			stepRequest.fetch_add(1);
			ticks.fetch_add(1);
		}
	}

	void service() {
		if (stepRequest.exchange(0) > 0)
			step();
		if (Flags & kChurn)
			churn();
	}

	void step() {
		if (Flags & kProbes) {
			for (const char *p : {"sdc:/sdc:/ChurnTestMissing/A",
								  "sdc:/sdc:/ChurnTestMissing",
								  "sdc:/sdc:/ChurnTestMissingToo",
								  "usb:/usb:/ChurnTestMissing/A",
								  "usb:/usb:/ChurnTestMissing",
								  "usb:/usb:/ChurnTestMissingToo"})
			{
				if (DIR *d = opendir(p))
					closedir(d);
			}
		}
		if (Flags & (kList | kRead))
			listRoot();
		if (Flags & kRead)
			readEntries();
		unsigned n = steps.fetch_add(1) + 1;
		if (n % 100 == 0)
			printf("[churn-test] flags %u: %u steps, %u names, %u entries\n",
				   Flags,
				   n,
				   (unsigned)names.size(),
				   (unsigned)entries.size());
		// The next entry becomes current, as a preset step would
		if (!entries.empty())
			std::snprintf(current, sizeof current, "%s", entries[n % entries.size()].name.c_str());
		else if (!names.empty())
			std::snprintf(current, sizeof current, "%s", names[n % names.size()].c_str());
	}

	void listRoot() {
		names.clear();
		if (DIR *d = opendir("sdc:/sdc:/")) {
			while (dirent *e = readdir(d)) {
				if (!e->d_name[0])
					break;
				names.push_back(e->d_name);
			}
			closedir(d);
		}
	}

	void readEntries() {
		entries.clear();
		for (const std::string &n : names) {
			if (n.size() < 6 || n[0] == '.' || n.compare(n.size() - 5, 5, ".json") != 0)
				continue;
			std::string path = "sdc:/" + n;
			FILE *f = std::fopen(path.c_str(), "rb");
			if (!f)
				continue;
			std::string text;
			char buf[1024];
			size_t got;
			while ((got = std::fread(buf, 1, sizeof buf, f)) > 0 && text.size() < 65536)
				text.append(buf, got);
			std::fclose(f);
			json_error_t error;
			json_t *rootJ = json_loads(text.c_str(), 0, &error);
			Entry entry;
			entry.name = n.substr(0, n.size() - 5);
			entry.file = n;
			if (rootJ) {
				if (json_t *j = json_object_get(rootJ, "algorithm"); j && json_is_string(j))
					entry.name += json_string_value(j);
				json_decref(rootJ);
			}
			entries.push_back(entry);
		}
	}

	// As Weaver's matchingPreset + presetTitle.set, every pass
	void churn() {
		char buf[256];
		std::memcpy(buf, current, sizeof buf);
		buf[255] = 0;
		std::string cur = buf;
		std::string match;
		for (const Entry &e : entries)
			if (e.name == cur) {
				match = e.name;
				break;
			}
		std::string title = match.empty() ? cur : match;
		std::snprintf(shownTitle, sizeof shownTitle, "%s", title.c_str());
	}

	std::atomic<int> stepRequest{0};
	std::atomic<unsigned> steps{0}, ticks{0};
	unsigned samples = 0;
	char current[256] = {};
	char shownTitle[256] = {};
	std::vector<std::string> names;
	std::vector<Entry> entries;
	MetaModule::AsyncThread thread{this, [this]() { service(); }};
};

// Text built in std::strings and a moving line, redrawn every frame
template<unsigned Flags>
struct ChurnDisplay : Widget {
	ChurnModule<Flags> *module = nullptr;
	int frame = 0;

	void draw(const DrawArgs &args) override {
		frame++;
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
		nvgFillColor(args.vg, nvgRGB(0x10, 0x18, 0x20));
		nvgFill(args.vg);

		float x = (frame % 50) / 50.f * box.size.x;
		nvgBeginPath(args.vg);
		nvgMoveTo(args.vg, x, 0);
		nvgLineTo(args.vg, box.size.x - x, box.size.y);
		nvgStrokeColor(args.vg, nvgRGB(0x40, 0xc0, 0xff));
		nvgStrokeWidth(args.vg, 1.5f);
		nvgStroke(args.vg);

		std::shared_ptr<window::Font> font =
			APP->window->loadFont(asset::system("res/fonts/ShareTechMono-Regular.ttf"));
		if (!font)
			return;
		nvgFontFaceId(args.vg, font->handle);
		nvgFontSize(args.vg, 9);
		nvgFillColor(args.vg, nvgRGB(0xff, 0xff, 0xff));
		unsigned steps = module ? module->steps.load() : 0;
		std::string line1 = "STEP " + std::to_string(steps);
		std::string line2 = module ? std::string(module->shownTitle) : std::string("preview");
		std::string line3 = "FRAME " + std::to_string(frame);
		nvgText(args.vg, 2, 10, line1.c_str(), nullptr);
		nvgText(args.vg, 2, 22, line2.substr(0, 12).c_str(), nullptr);
		nvgText(args.vg, 2, 34, line3.c_str(), nullptr);
	}
};

template<unsigned Flags>
struct ChurnWidget : ModuleWidget {
	ChurnWidget(ChurnModule<Flags> *module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/panel.svg")));
		if (Flags & kDisplay) {
			for (int i = 0; i < 2; i++) {
				auto *d = createWidget<ChurnDisplay<Flags>>(mm2px(Vec(1.f, 10.f + i * 50.f)));
				d->box.size = mm2px(Vec(18.f, 40.f));
				d->module = module;
				addChild(d);
			}
		}
	}
};

template<unsigned Flags>
Model *model(const char *slug) {
	return createModel<ChurnModule<Flags>, ChurnWidget<Flags>>(slug);
}

} // namespace

void init(Plugin *p) {
	pluginInstance = p;
	p->addModel(model<(kChurn | kProbes | kList | kRead)>("All"));
	p->addModel(model<(kProbes | kList | kRead)>("NoChurn"));
	p->addModel(model<(kChurn)>("ChurnOnly"));
	p->addModel(model<(kProbes)>("ProbesOnly"));
	p->addModel(model<(kList | kRead)>("ListRead"));
	p->addModel(model<(kChurn | kProbes | kList | kRead | kDisplay)>("AllDisplay"));
	p->addModel(model<(kDisplay)>("DisplayOnly"));
	p->addModel(model<(kChurn | kDisplay)>("ChurnDisplay"));
	p->addModel(model<(kProbes | kDisplay)>("ProbesDisplay"));
	p->addModel(model<(kList | kRead | kDisplay)>("ListDisplay"));
}
