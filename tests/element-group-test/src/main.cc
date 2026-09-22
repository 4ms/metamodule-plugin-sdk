// Test plugin for element groups.
//
// The modules here have no group code at all: their groups are declared in this
// plugin's plugin-mm.json, and each module exercises one way of naming elements.
//
// Group Filter:  members named by the element name shown on screen.
// Group Refs:    members named by typed index (param:N, in:N, out:N), the form a
//                module ported from VCV Rack ends up with. Its "Broken" group
//                deliberately names two ids that don't exist: the group should still
//                work, with the bad members dropped and a warning logged.
// Group Decl:    members named by the info struct's Elem enumerators, which the SDK
//                rewrites to elem:N at package time using the module's DWARF info.

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"

#include <array>
#include <cmath>
#include <cstdio>
#include <string_view>

namespace
{

using namespace MetaModule;

constexpr std::string_view Brand{"ElementGroupTest"};

constexpr std::string_view KnobImage{"4ms/comp/knob_x.png"};
constexpr std::string_view JackImage{"4ms/comp/jack_x.png"};

// Elements are built field-by-field rather than with nested aggregate braces,
// to keep the element lists below readable
template<typename T>
constexpr T make_element(float x, float y, std::string_view name, std::string_view image) {
	T el{};
	el.x_mm = x;
	el.y_mm = y;
	el.coords = Coords::Center;
	el.short_name = name;
	el.long_name = name;
	el.image = image;
	return el;
}

constexpr Element knob(float x, float y, std::string_view name) {
	return make_element<Knob>(x, y, name, KnobImage);
}

constexpr Element jack_in(float x, float y, std::string_view name) {
	return make_element<JackInput>(x, y, name, JackImage);
}

constexpr Element jack_out(float x, float y, std::string_view name) {
	return make_element<JackOutput>(x, y, name, JackImage);
}

// A plain mixer: every knob attenuates its input, and the sum goes to the output.
// The point of these modules is the element list, not the DSP.
template<size_t NumKnobs, size_t NumIns>
class SumModule : public CoreProcessor {
public:
	void update() override {
		float sum = 0.f;
		for (auto i = 0u; i < NumIns; i++)
			sum += inputs[i] * params[i < NumKnobs ? i : 0];
		output = sum;
	}

	void set_param(int id, float val) override {
		if (id >= 0 && (size_t)id < NumKnobs)
			params[id] = val;
	}

	void set_input(int id, float val) override {
		if (id >= 0 && (size_t)id < NumIns)
			inputs[id] = val;
	}

	float get_output(int) const override {
		return output;
	}

	void set_samplerate(float) override {
	}

private:
	std::array<float, NumKnobs> params{};
	std::array<float, NumIns> inputs{};
	float output = 0.f;
};

//
// Group Filter: groups refer to elements by the Elem enum
//

struct GroupFilterInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"GroupFilter"};
	static constexpr std::string_view description{"Element groups by element index"};
	static constexpr uint32_t width_hp = 12;
	static constexpr std::string_view png_filename{"ElementGroupTest/panel.png"};

	static constexpr std::array<Element, 15> Elements{{
		knob(15.f, 20.f, "Volume"),

		knob(30.f, 20.f, "Cutoff"),
		knob(45.f, 20.f, "Resonance"),
		knob(15.f, 35.f, "Filter Mode"),

		knob(30.f, 35.f, "Attack"),
		knob(45.f, 35.f, "Decay"),
		knob(15.f, 50.f, "Sustain"),
		knob(30.f, 50.f, "Release"),

		knob(45.f, 50.f, "LFO Rate"),
		knob(15.f, 65.f, "LFO Depth"),

		knob(30.f, 65.f, "Output Gain"),

		jack_in(15.f, 100.f, "Left"),
		jack_in(30.f, 100.f, "Right"),
		jack_in(45.f, 100.f, "Gate"),
		jack_out(15.f, 115.f, "Out"),
	}};

	enum class Elem {
		VolumeKnob,
		CutoffKnob,
		ResonanceKnob,
		FilterModeKnob,
		AttackKnob,
		DecayKnob,
		SustainKnob,
		ReleaseKnob,
		LfoRateKnob,
		LfoDepthKnob,
		OutputGainKnob,
		LeftIn,
		RightIn,
		GateIn,
		Out,
	};
};

static_assert(ElementCount::count<GroupFilterInfo>() ==
			  ElementCount::Counts{.num_params = 11, .num_lights = 0, .num_inputs = 3, .num_outputs = 1});

//
// Group Refs: groups refer to elements by param/jack id, the way a module ported
// from VCV Rack has to
//

struct GroupRefsInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"GroupRefs"};
	static constexpr std::string_view description{"Element groups by param and jack id"};
	static constexpr uint32_t width_hp = 12;
	static constexpr std::string_view png_filename{"ElementGroupTest/panel.png"};

	static constexpr std::array<Element, 10> Elements{{
		knob(15.f, 20.f, "Volume"),	  // param 0
		knob(30.f, 20.f, "Shape"),	  // param 1
		knob(45.f, 20.f, "Fold"),	  // param 2
		knob(15.f, 35.f, "Symmetry"), // param 3
		knob(30.f, 35.f, "Drive"),	  // param 4
		knob(45.f, 35.f, "Bias"),	  // param 5

		jack_in(15.f, 100.f, "In"),		 // input 0
		jack_in(30.f, 100.f, "Shape CV"), // input 1
		jack_in(45.f, 100.f, "Fold CV"),  // input 2
		jack_out(15.f, 115.f, "Out"),	  // output 0
	}};

	// Stand-ins for the ParamIds / InputIds / OutputIds enums a VCV module has
	enum ParamId { VOLUME_PARAM, SHAPE_PARAM, FOLD_PARAM, SYMMETRY_PARAM, DRIVE_PARAM, BIAS_PARAM };
	enum InputId { MAIN_INPUT, SHAPE_CV_INPUT, FOLD_CV_INPUT };
	enum OutputId { MAIN_OUTPUT };
};

static_assert(ElementCount::count<GroupRefsInfo>() ==
			  ElementCount::Counts{.num_params = 6, .num_lights = 0, .num_inputs = 3, .num_outputs = 1});

//
// Group Decl: groups declared in the info struct
//

struct GroupDeclInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"GroupDecl"};
	static constexpr std::string_view description{"Element groups declared in the info struct"};
	static constexpr uint32_t width_hp = 12;
	static constexpr std::string_view png_filename{"ElementGroupTest/panel.png"};

	static constexpr std::array<Element, 8> Elements{{
		knob(15.f, 20.f, "Level"),
		knob(30.f, 20.f, "Delay Time"),
		knob(45.f, 20.f, "Feedback"),
		knob(15.f, 35.f, "Delay Mix"),
		knob(30.f, 35.f, "Reverb Size"),
		knob(45.f, 35.f, "Reverb Mix"),

		jack_in(15.f, 100.f, "In"),
		jack_out(15.f, 115.f, "Out"),
	}};

	enum class Elem {
		LevelKnob,
		DelayTimeKnob,
		FeedbackKnob,
		DelayMixKnob,
		ReverbSizeKnob,
		ReverbMixKnob,
		In,
		Out,
	};
};

static_assert(ElementCount::count<GroupDeclInfo>() ==
			  ElementCount::Counts{.num_params = 6, .num_lights = 0, .num_inputs = 1, .num_outputs = 1});

using GroupFilter = SumModule<11, 3>;
using GroupRefs = SumModule<6, 3>;

// Uses its Elem enum to route params. The enum has to be used by the code for the
// compiler to emit it into DWARF, which is what the SDK reads to rewrite the
// enumerator names in this plugin's plugin-mm.json.
class GroupDecl : public SumModule<6, 1> {
	using Elem = GroupDeclInfo::Elem;

public:
	void set_param(int id, float val) override {
		switch (Elem(id)) {
			case Elem::LevelKnob:
				level = val;
				break;
			case Elem::DelayTimeKnob:
				delay_time = val;
				break;
			default:
				break;
		}
		SumModule::set_param(id, val);
	}

private:
	float level = 0.f;
	float delay_time = 0.f;
};

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	using namespace MetaModule;

	register_module<GroupFilter, GroupFilterInfo>(Brand);
	register_module<GroupRefs, GroupRefsInfo>(Brand);
	register_module<GroupDecl, GroupDeclInfo>(Brand);

	printf("[element-group-test] modules registered; groups come from plugin-mm.json\n");
}
