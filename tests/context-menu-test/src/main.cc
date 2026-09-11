// Test plugin for the native module context menu API (CoreModules/context_menu.hh)
//
// Menu LFO:
//   Registered with the member-function helper register_context_menu<ModuleT, ModuleInfoT>().
//   Its menu uses every row type: Label, Checkbox, Divider, Action, and Slider.
//   The menu settings change the LFO output, and are saved with the patch
//   (save_state/load_state). Add two Menu LFOs to a patch to verify each
//   instance's menu shows and changes only its own settings.
//
// Menu Gain:
//   Registered with a ContextMenuHandlers struct of lambdas (the non-template form).
//   A Slider sets the gain (0x to 2x) and a Checkbox mutes the output.
//
// init() also verifies that registering a menu for a module that isn't
// registered (yet) is rejected.

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/context_menu.hh"
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"

#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace
{

using namespace MetaModule;

constexpr std::string_view Brand{"ContextMenuTest"};

std::string percent_text(float val) {
	return std::to_string(int(std::lround(val * 100.f))) + "%";
}

//
// Menu LFO
//

struct MenuLfoInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"MenuLFO"};
	static constexpr std::string_view description{"LFO configured from its context menu"};
	static constexpr uint32_t width_hp = 6;
	static constexpr std::string_view png_filename{"ContextMenuTest/panel.png"};

	static constexpr std::array<Element, 2> Elements{{
		MonoLight{{{{15.24f, 26.f, Coords::Center, "LFO"}, "4ms/comp/led_x.png"}}, Colors565::Green},
		JackOutput{{{{15.24f, 36.f, Coords::Center, "Out"}, "4ms/comp/jack_x.png"}}},
	}};
};

static_assert(ElementCount::count<MenuLfoInfo>() ==
			  ElementCount::Counts{.num_params = 0, .num_lights = 1, .num_inputs = 0, .num_outputs = 1});

class MenuLfo : public CoreProcessor {
	enum class Wave : unsigned { Sine, Triangle, Square };
	static constexpr unsigned NumWaves = 3;

	// Row indices, in the order that menu_items() returns them
	enum MenuRow : unsigned { Heading, Invert, Fast, Separator, WaveSelect, Level, Reset };

public:
	std::vector<ContextMenuItem> menu_items() {
		using Type = ContextMenuItem::Type;
		return {
			{.type = Type::Label, .name = "LFO Options"},
			{.type = Type::Checkbox, .name = "Invert", .checked = invert},
			{.type = Type::Checkbox, .name = "Fast (x4)", .checked = fast},
			{.type = Type::Divider},
			{.type = Type::Action, .name = "Wave:", .value_text = wave_name()},
			{.type = Type::Slider, .name = "Level", .value = level, .value_text = percent_text(level)},
			{.type = Type::Action, .name = "Reset to defaults"},
		};
	}

	void menu_changed(unsigned index, float value) {
		switch (index) {
			case Invert:
				invert = value > 0.5f;
				break;
			case Fast:
				fast = value > 0.5f;
				break;
			case WaveSelect:
				wave = Wave((unsigned(wave) + 1) % NumWaves);
				break;
			case Level:
				level = value;
				break;
			case Reset:
				invert = false;
				fast = false;
				wave = Wave::Sine;
				level = 1.f;
				break;
			default:
				break;
		}
	}

	void update() override {
		phase += (fast ? 4.f : 1.f) / samplerate;
		if (phase >= 1.f)
			phase -= 1.f;

		float out = waveform(phase) * level;
		output = invert ? -out : out;
	}

	float get_output(int) const override {
		return output * 5.f;
	}

	float get_led_brightness(int) const override {
		return output > 0.f ? output : 0.f;
	}

	void set_samplerate(float sr) override {
		samplerate = sr;
	}

	void set_param(int, float) override {
	}

	void set_input(int, float) override {
	}

	// Menu settings persist with the patch.
	// Format: invert, fast, wave, level% -- e.g. "0,1,2,75"
	std::string save_state() override {
		return std::to_string(invert) + "," + std::to_string(fast) + "," + std::to_string(unsigned(wave)) + "," +
			   std::to_string(int(std::lround(level * 100.f)));
	}

	void load_state(std::string_view state) override {
		std::array<int, 4> vals{};
		auto ptr = state.data();
		auto end = state.data() + state.size();
		for (auto &val : vals) {
			auto [next, err] = std::from_chars(ptr, end, val);
			if (err != std::errc{})
				return;
			ptr = (next < end && *next == ',') ? next + 1 : next;
		}
		invert = vals[0];
		fast = vals[1];
		wave = Wave(unsigned(vals[2]) % NumWaves);
		level = std::clamp(vals[3] / 100.f, 0.f, 1.f);
	}

private:
	float waveform(float ph) const {
		switch (wave) {
			case Wave::Triangle:
				return ph < 0.5f ? (4.f * ph - 1.f) : (3.f - 4.f * ph);
			case Wave::Square:
				return ph < 0.5f ? 1.f : -1.f;
			case Wave::Sine:
			default:
				return std::sin(ph * 6.2831853f);
		}
	}

	std::string wave_name() const {
		switch (wave) {
			case Wave::Triangle:
				return "Triangle";
			case Wave::Square:
				return "Square";
			case Wave::Sine:
			default:
				return "Sine";
		}
	}

	// Menu state: written by the GUI context, read by update()
	bool invert = false;
	bool fast = false;
	Wave wave = Wave::Sine;
	float level = 1.f;

	float samplerate = 48000.f;
	float phase = 0.f;
	float output = 0.f;
};

//
// Menu Gain
//

struct MenuGainInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"MenuGain"};
	static constexpr std::string_view description{"Gain configured from its context menu"};
	static constexpr uint32_t width_hp = 6;
	static constexpr std::string_view png_filename{"ContextMenuTest/panel.png"};

	static constexpr std::array<Element, 2> Elements{{
		JackInput{{{{15.24f, 88.f, Coords::Center, "In"}, "4ms/comp/jack_x.png"}}},
		JackOutput{{{{15.24f, 106.f, Coords::Center, "Out"}, "4ms/comp/jack_x.png"}}},
	}};
};

static_assert(ElementCount::count<MenuGainInfo>() ==
			  ElementCount::Counts{.num_params = 0, .num_lights = 0, .num_inputs = 1, .num_outputs = 1});

class MenuGain : public CoreProcessor {
public:
	void update() override {
	}

	void set_input(int, float val) override {
		input = val;
	}

	float get_output(int) const override {
		return mute ? 0.f : input * gain;
	}

	void set_samplerate(float) override {
	}

	void set_param(int, float) override {
	}

	float gain = 1.f; // 0..2
	bool mute = false;

private:
	float input = 0.f;
};

ContextMenuHandlers make_gain_menu() {
	return {
		.get_items =
			[](CoreProcessor *module) {
				auto gain_module = static_cast<MenuGain *>(module);

				char gain_text[16];
				snprintf(gain_text, sizeof gain_text, "%.2fx", gain_module->gain);

				using Type = ContextMenuItem::Type;
				return std::vector<ContextMenuItem>{
					{.type = Type::Slider, .name = "Gain", .value = gain_module->gain / 2.f, .value_text = gain_text},
					{.type = Type::Checkbox, .name = "Mute", .checked = gain_module->mute},
				};
			},

		.on_change =
			[](CoreProcessor *module, unsigned index, float value) {
				auto gain_module = static_cast<MenuGain *>(module);
				if (index == 0)
					gain_module->gain = value * 2.f;
				else if (index == 1)
					gain_module->mute = value > 0.5f;
			},
	};
}

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	using namespace MetaModule;

	// A menu can't be registered before its module
	bool early_ok = register_context_menu(Brand, MenuGainInfo::slug, make_gain_menu());

	register_module<MenuLfo, MenuLfoInfo>(Brand);
	register_module<MenuGain, MenuGainInfo>(Brand);

	bool lfo_ok = register_context_menu<MenuLfo, MenuLfoInfo>(Brand, &MenuLfo::menu_items, &MenuLfo::menu_changed);
	bool gain_ok = register_context_menu(Brand, MenuGainInfo::slug, make_gain_menu());

	printf("[context-menu-test] early registration rejected: %s\n", early_ok ? "FAIL" : "pass");
	printf("[context-menu-test] MenuLFO menu registered: %s\n", lfo_ok ? "pass" : "FAIL");
	printf("[context-menu-test] MenuGain menu registered: %s\n", gain_ok ? "pass" : "FAIL");
}
