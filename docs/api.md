### CoreProcessor

All modules must derive from this class.

```c++
class CoreProcessor {
public:
    virtual void update() = 0;
    virtual void set_samplerate(float sr) = 0;
    virtual void set_param(int param_id, float val) = 0;
    virtual void set_input(int input_id, float val) = 0;
    virtual float get_output(int output_id) const = 0;
    virtual float get_led_brightness(int led_id) { return 0; }
    virtual size_t get_display_text(int display_id, std::span<char> text) { return 0; }
    virtual void mark_all_inputs_unpatched() {}
    virtual void mark_input_unpatched(int input_id) {}
    virtual void mark_input_patched(int input_id) {}
    virtual void mark_all_outputs_unpatched() {}
    virtual void mark_output_unpatched(int output_id) {}
    virtual void mark_output_patched(int output_id) {}
    virtual void load_state(std::string_view state_data) {}
    virtual std::string save_state() { return ""; }
};
```

#### Called in the audio task:

The following functions are called by the audio engine and thus have hard
real-time requirements. These must not allocate memory (no malloc,
vector.push_back, string.resize(), etc.).

`void update()`: Called at sample rate on every module in the patch. The main processing code should go here: calculate jack and light outputs based on jack and param inputs.

`void set_samplerate(float sr)` Called when a module is loaded and whenever the user changes the samplerate.

`void set_param(int param_id, float val)` Called by the audio engine to set the value of a knob, button, switch, etc. The value `val` will be in the range 0..1. The function must check if `param_id` is a valid value, and do nothing if it's invalid. 


### Registering a Module:

```c++

namespace MetaModule {

template <typename ModuleT, ModuleInfoT>
bool register_module(std::string_view brand_name, 
                     std::string_view module_slug,
                     std::string_view faceplate_filename);


```


### ModuleInfoBase

ModuleInfoBase is the base class for ModuleInfo structs which define the 
characteristics of all module elements (knobs, jacks, lights, etc) and other
meta data. Modules should create their own ModuleInfo class that derives from
this.

```c++
namespace MetaModule {

struct ModuleInfoBase {
    static constexpr std::string_view slug{""};
    static constexpr std::string_view description{""};
    static constexpr uint32_t width_hp = 0;
    static constexpr std::array<Element, 0> Elements{};
    static constexpr std::string_view svg_filename{""};
    static constexpr std::string_view png_filename{""};

    struct BypassRoute {
        uint16_t input;
        uint16_t output;
    };
    static constexpr std::array<BypassRoute, 0> bypass_routes{};
};


}
```


`slug`: this is the exact name used to identify the module. If porting from VCV Rack, it must match the VCV module slug.

`description`: currently not used.

`width_hp`: currently not used, but must not be 0.

`svg_filename`: used only by 4ms modules for the 4ms VCV plugin.

`png_filename': the path to the PNG file used for the faceplate.

`bypass_routes`: Optional. Specify an array of input->output paths, providing the element ID for each.

`Elements`: this is an array of elements such as jacks, knobs, etc. 
Each entry is of the type `Element` which is a std::variant type of various base types:

```c++
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

                             AltParamContinuous,
                             AltParamChoice,
                             AltParamChoiceLabeled>;
```

You may use these types (Knob, Slider, RgbLight, etc) or create your own types that derive from one of these. Each type has different fields which describe it. For example, `Knob` has fields for the angles that it should be drawn as the user turns the knob:

```c++
struct Knob : Pot {
    // How much to rotate the image when param val == 0
    // negative is CCW, positive is CW
    float min_angle = -135.f;
    // How much to rotate the image when param val == 1
    float max_angle = 135.f;
};
```

`Knob` inherits from `Pot`, which has more fields:

```c++
struct Pot : ParamElement {
    State_t default_value = 0.5f;
    State_t min_value = 0.f;
    State_t max_value = 1.f;
    float display_base = 0.f;
    float display_mult = 1.f;
    float display_offset = 0.f;
    std::string_view units = "";
    bool integral = false;
    uint8_t display_precision = 0;
};

```

`Pot` inherits from `ParamElement` which in turn inherits from `ImageElement`:

```c++
struct ImageElement : BaseElement {
    std::string_view image = "";
};
```

`image`: This is where the path to the PNG image for the knob is specified.

Finally, we get to the base class `BaseElement`:

```c++
struct BaseElement {
    float x_mm = 0;
    float y_mm = 0;
    Coords coords = Coords::Center;

    std::string_view short_name;
    std::string_view long_name;

    float width_mm = 0;
    float height_mm = 0;
};
```

`x_mm`, `y_mm`: This specifies the x,y position of where to draw the element.


`coords` This can be Coords::Center to indicate x,y is the center of the
element; or it can be Coords::TopLeft to indicate x,y is the top-left of the
element.

`short_name`: The display name of the element.

`long_name`: not used.

`width_mm`, `height_mm`: Not usually used since the image itself determines the dimensions, but in case the image file cannot be found then these are used.


Please take a look at `CoreModules/elements/base_element.hh` for the complete list of types and their fields.


#### Elements array

The order of the elements in the Elements array determines the ID of each param, jack, and light.
The first param has ID 0, the second param has ID 1, etc...

To make this easy, the ElementCount namespace provides several ways to get the index of an entry in the Elements array.


