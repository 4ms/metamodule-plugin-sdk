#pragma once
#include "base_element.hh"
#include <variant>

namespace MetaModule
{

// Criteria for being a member of the variant.
// - Unique NumParams/NumLights/NumInputs/NumOutputs values (because these are static constexpr).
// - Unique data members (e.g. SlideSwitch::num_pos)
// - Unique animation or drawing method (e.g.: Having two types Slider and Knob is preferred to having Pot::is_slider).
// - Unique set of values or method of interpreting values (e.g. Momentary vs Latched buttons)

using Element = std::variant<NullElement,
							 ImageElement,
							 ParamElement,

							 Knob,
							 Slider,
							 SliderLight,

							 FlipSwitch,
							 SlideSwitch,

							 MomentaryButton,
							 MomentaryButtonLight,
							 MomentaryButtonRGB,

							 LatchingButton,

							 Encoder,
							 EncoderRGB,

							 JackInput,
							 JackOutput,

							 MonoLight,
							 DualLight,
							 RgbLight,

							 TextDisplay,
							 DynamicTextDisplay,
							 DynamicGraphicDisplay,

							 AltParamContinuous,
							 AltParamChoice,
							 AltParamChoiceLabeled,
							 AltParamAction,

							 KnobSnapped>;

namespace
{
inline constexpr auto ElementSize = sizeof(Element);
}

// helper:
inline BaseElement base_element(const Element &el) {
	if (el.index() == Element{NullElement{}}.index())
		return BaseElement{};
	else
		return std::visit([](auto e) { return BaseElement{e}; }, el);
}
} // namespace MetaModule
