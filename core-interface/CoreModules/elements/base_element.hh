#pragma once
#include "util/colors_rgb565.hh"
#include <array>
#include <cstdint>
#include <string_view>

// Hierarchy:
//                                                   BaseElement
//                     +---------------------------------'--------------+---------------------------+-------------------------------+
//                     |                                                |                           |                               |
//              ParamElement                                        JackElement                  LightElement                   AltParam
//     +----------+---'--------+------------------+                +------'-----+         +---------+'-------+---------+
//     |          |            |                  |                |            |         |         |        |         |
//    Pot     Encoder       Switch              Button          JackInput  JackOutput  MonoLight DualLight RGBLight Display
//  +--'--+        |     +----'---+          +----'---------+
//  |     |        |     |        |          |              |
// Knob  Slider    |   FlipSw  SlideSw    MomBut         LatchingBut
//        |        |                     +---'---+          |
//        |        |                     |       |          |
//   SliderLight  EncoderRGB      MomButWhite MomButRGB   LatButMonoLight
//

namespace MetaModule
{

enum class Coords { Center, TopLeft };

struct BipolarColor_t {
	float value;
};

struct FullColor_t {
	float value;
};

struct BaseElement {
	float x_mm = 0;
	float y_mm = 0;
	Coords coords = Coords::Center;

	std::string_view short_name;
	std::string_view long_name;

	float width_mm = 0;
	float height_mm = 0;

	static constexpr size_t NumParams = 0;
	static constexpr size_t NumLights = 0;
	static constexpr size_t NumInputs = 0;
	static constexpr size_t NumOutputs = 0;

	// This needs to be there to catch no State_t defined by children
	using State_t = void;
};

struct ImageElement : BaseElement {
	std::string_view image = "";
};

struct NullElement : BaseElement {};

// ParamElement: base class for pot, encoder, switch/button
struct ParamElement : ImageElement {
	static constexpr size_t NumParams = 1;
	static constexpr float default_value = 0;
};

// Pots (Knobs, Sliders)
struct Pot : ParamElement {
	using State_t = float;
	State_t default_value = 0.5f;
	State_t min_value = 0.f;
	State_t max_value = 1.f;
	float display_base = 0.f;
	float display_mult = 1.f;
	float display_offset = 0.f;
	std::string_view units = "";
	bool unused = false;
	uint8_t display_precision = 0;
};

struct Knob : Pot {
	// How much to rotate the image when param val == 0
	// negative is CCW, positive is CW
	float min_angle = -135.f;
	// How much to rotate the image when param val == 1
	float max_angle = 135.f;
};

struct KnobSnapped : Knob {
	constexpr static size_t MaxPosNames = 30;
	unsigned num_pos = 0;
	std::array<const char *, MaxPosNames> pos_names{};
};

struct Slider : Pot {
	std::string_view image_handle = "";
};

struct SliderLight : Slider {
	static constexpr size_t NumLights = 1;
	RGB565 color = Colors565::White;
};

//
// Buttons
//
struct Button : ParamElement {};

struct MomentaryButton : Button {
	enum class State_t { RELEASED, PRESSED };
	std::string_view pressed_image = "";
	static constexpr State_t default_value = State_t::RELEASED;
};

struct MomentaryButtonRGB : MomentaryButton {
	static constexpr size_t NumLights = 3;
};
struct MomentaryButtonLight : MomentaryButton {
	static constexpr size_t NumLights = 1;
	RGB565 color = Colors565::White;
};

// LatchingButton always has a single color LED
// It's drawn with a single frame, and the color is applied as a filled circle,
// whose alpha value equals the LED value
struct LatchingButton : Button {
	enum class State_t { UP, DOWN };
	State_t default_value = State_t::UP;
	static constexpr size_t NumLights = 1;
	RGB565 color = Colors565::White;
};

//
// Switches
//
struct Switch : ParamElement {
	using State_t = unsigned;
	unsigned num_pos = 3;
	State_t default_value = 0;
};

// FlipSwitch has up to 4 frames
// Frame n is drawn to indicate value == n/(num_pos-1)
struct FlipSwitch : Switch {
	static constexpr size_t MaxPositions = 10;
	std::array<std::string_view, MaxPositions> frames{};
	std::array<std::string_view, MaxPositions> pos_names{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
};

// SlideSwitch has a bg (body) image and a fg (handle) image
// The handle is drawn at evenly spaced positions to indicate the switch's value
struct SlideSwitch : Switch {
	std::string_view image_handle = "";
	enum class Ascend { UpLeft, DownRight } direction = Ascend::DownRight;
	std::array<std::string_view, 8> pos_names{};
};

// Encoders
struct Encoder : ParamElement {};

struct EncoderRGB : Encoder {
	static constexpr size_t NumLights = 3;
};

// Jacks
struct JackElement : ImageElement {};

struct JackInput : JackElement {
	static constexpr size_t NumInputs = 1;
};

struct JackOutput : JackElement {
	static constexpr size_t NumOutputs = 1;
};

// Lights
struct LightElement : ImageElement {};

struct MonoLight : LightElement {
	static constexpr size_t NumLights = 1;
	RGB565 color = Colors565::White;
};

struct DualLight : LightElement {
	static constexpr size_t NumLights = 2;
	std::array<RGB565, 2> color = {Colors565::White, Colors565::White};
};
struct RgbLight : LightElement {
	static constexpr size_t NumLights = 3;
};

// Displays
struct TextDisplay : LightElement {
	std::string_view text = "";
	std::string_view font = "";
	RGB565 color = Colors565::Grey;
	enum class WrapMode { Clip, Wrap, Scroll, ScrollBounce };
	WrapMode wrap_mode = WrapMode::Clip;
};

struct DynamicTextDisplay : TextDisplay {
	static constexpr size_t NumLights = 1;
};

struct DynamicGraphicDisplay : LightElement {
	static constexpr size_t NumLights = 1;
	bool full_module = false;
};

// AltParams:
// Like Params but they are not drawn on the faceplate (access by menu only)
struct AltParamElement : BaseElement {
	static constexpr size_t NumParams = 1;
};

struct AltParamContinuous : AltParamElement {
	using State_t = float;
	State_t default_value = 0.5f;
	State_t MinValue = 0.0f;
	State_t MaxValue = 1.0f;
};

struct AltParamChoice : AltParamElement {
	using State_t = unsigned;

	unsigned num_pos = 2;
	State_t default_value = 0;
};

struct AltParamAction : AltParamElement {
	// clicking this performs an action
	using State_t = bool;
};

struct AltParamChoiceLabeled : AltParamChoice {
	static constexpr size_t MaxChoices = 8;
	std::array<std::string_view, MaxChoices> pos_names{};
};

} // namespace MetaModule
