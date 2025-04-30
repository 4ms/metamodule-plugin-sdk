# MetaModule SDK overview

This document provides an overview of the MetaModule plugin SDK.
It focuses on help for creating native CoreModule plugins, also
called "Native" plugins.
When porting modules from VCV Rack, the metamodule-rack-interface
layer interfaces between the VCV Rack code and the plugin SDK's API.
However, it still may be useful to read this to understand what's
going on behind the scenes.

## CoreProcessor

All modules must derive from this class.

```c++
class CoreProcessor {
public:
    virtual void update() = 0;
    virtual void set_samplerate(float sr) = 0;
    virtual void set_param(int param_id, float val) = 0;
    virtual float get_param(int param_id) const = 0;
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

### Real-time functions (called by the audio engine):

The following functions are called by the audio engine and thus have hard
real-time requirements. These must not allocate memory (no malloc,
vector.push_back(), string.resize(), etc.).

- `void update()`: Called at sample rate on every module in the patch. The main
processing code should go here: calculate jack and light outputs based on jack
and param inputs.

- `void set_samplerate(float sr)` Called when a module is loaded and whenever the
user changes the samplerate.

- `void set_param(int param_id, float val)` Called by the audio engine to set the
value of a knob, button, switch, etc. The value `val` will be in the range
0..1. The function must check if `param_id` is a valid value, and do nothing if
it's invalid. 

- `void get_param(int param_id)` Called by the audio engine and GUI engine to get
the value of a knob, button, switch, etc. Must return 0..1. The function must
check if `param_id` is a valid value, and should return 0 if it's invalid. Note
that the value returned by `get_param` might be different than the value the
engine assigned in `set_param` if the parameter has `integral` (aka "snap")
enabled, or some parameters interact with each other (e.g. a knob selects a
pattern which toggles the state of some buttons).

- `void set_input(int input_id, float val)`:  Called by the audio engine to send
a voltage to a jack. The value passed will be between -10 and +10, which refers
to a voltage level. The function must check if input_id is a valid id, and do 
nothing if it's invalid.

- `float get_output(int output_id)`: Called by the audio engine to get the voltage
of an output jack. The value returned must be in the range -10 to +10.
If the output_id is not valid, the function must return 0.

- `void mark_input_unpatched(int input_id)`:
- `void mark_input_patched(int input_id)`:
- `void mark_output_unpatched(int output_id)`:
- `void mark_output_patched(int output_id)`: These functions are called by the
  audio engine to tell the module that one of its jacks has changed state from
  being patched to unpatched or vice-versa. This will be called only when a
  patch is first loaded, a module is added to a patch, a user physically plugs
  or unplugs a cable into a jack, or a user creates or removes a virtual cable.
  The function must check if input_id or output_id is valid, and do nothing if
  not. Some modules may choose to ignore these, other modules may wish to use
  these for normalizations or changing state depending on the patch
  configuration.

- `void mark_all_inputs_unpatched()`:
- `void mark_all_outputs_unpatched()`: These have the same effect as above but
  are only called when the module is first loaded into a patch. It's likely the
  calls to these will be followed up by calls to individual jacks to mark them
  as patched.

### Non-realtime functions (called by the GUI engine):

The following functions are never called by the audio engine and therefore do
not have real-time requirements:

- `float get_led_brightness(int led_id)`: Called by the GUI engine to get the
  brightness of a light. Must return a value from 0 to 1, where 0 if off, and 1
  is full brightness. for RGB or dual-color lights, each element of the
  multi-color light will have its own led_id, so this function should return
  the brightness of one color component. This function is called every GUI
  update frame (max every 16ms). Since there can be large numbers of LEDs in a
  patch, it's expected that this function runs quickly. So while it's
  technically OK to allocate memory or perform complex calculations in this
  function, it will give users a bad experience (slow frame rate, laggy UI) and
  is not recommended.

- `size_t get_display_text(int display_id, std::span<char> text)`: This
  function is called by the GUI engine to get text that should be displayed in
  a DynamicTextDisplay element. If you module has no DynamicTextDisplay
  elements, then it will never be called and can be omitted from the class. The
  function should check if the display_id is valid and return 0 if not.
  Otherwise, it should fill the `text` parameter with text and return the
  number of bytes written. The function must not write characters past the end
  of the text span. That is, do not write more characters than the size of the
  span (`text.size()`). The characters written must be valid characters in the
  font of the DynamicTextDisplay. 
  It's OK to allocate memory in this function (e.g. use std::string). It's not
  recommended to make system calls such as reading from the filesystem, since
  it will make for a bad user experience (slow frame rate, laggy UI response).


- `void load_state(std::string_view state_data)`: This function is called when
  a module is first loaded or when its re-initialized (via a user clicking
  "Reset" in the Module Action menu). Modules are allowed to save their state
  into the patch file. They can use whatever format they wish as long as it can
  be represented by a std::string. Typically, json or base64 encoding are used.
  Since parameter values are saved in the patch file separately, load_state
  only needs to concern itself with any other state that won't be represented
  solely by parameter values. For example, if a module has tap tempo, then the
  tempo value should be saved and loaded. If a module chooses to implement this
  function, then it should parse/decode the state_data parameter and initialize
  its state to this value. If there is a parsing error or the data is invalid,
  then the error should not be reported and instead the module should
  initialize itself to a valid state. As a special-case, if an empty string is
  passed (state_data == "") then the module should reset to default parameter
  values and state. This function may allocate memory, make system calls, or
  perform expensive calculations. The audio context will be paused while this
  function executes.

- `std::string save_state()`: This is the complement of `load_state`. When a
  user saves a patch, then the engine will call this to get a string that
  represents the state of the module.

- Constructor and destructor: The audio thread will be paused (not playing)
  when these are called, so it's safe to perform memory allocations, make
  filesystem calls, or do expensive calculations. In many situations, the
  constructor is the ideal time to populate or reserve space for `std::vector`
  or other dynamic containers.


## ModuleInfoBase

ModuleInfoBase is the base class for ModuleInfo structs which define the 
characteristics of all module elements (knobs, jacks, lights, etc) and other
meta data. Modules should create their own ModuleInfo class that derives from
this.
(see [metamodule-core-interface/CoreModules/elements/element_info.hh])

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

`png_filename`: the path to the PNG file used for the faceplate. Must start with the brand slug, e.g. `4ms/faceplates/module.png`

`bypass_routes`: Optional. Specify an array of input->output paths, providing the element ID for each.

`Elements`: this is an array of elements such as jacks, knobs, etc. 
Each entry is of the type `Element` which is a std::variant type of various base types:
(see [metamodule-core-interface/CoreModules/elements/elements.hh] for the variant definition and 
[metamodule-core-interface/CoreModules/elements/base_element.hh] for the element definitions).



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

You may use these types (Knob, Slider, RgbLight, etc) or create your own types
that derive from one of these. Each type has different fields which describe
it. For example, `Knob` has fields for the angles that it should be drawn as
the user turns the knob:

```c++
struct Knob : Pot {
    // How much to rotate the image when param val == 0
    // 0 is noon, negative is CCW, positive is CW
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

Finally, we see that ImageElement inherits from the top base class `BaseElement`:

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

`width_mm`, `height_mm`: Not always used since the image itself determines the
dimensions, but in case the image file cannot be found then these are used.
Also used for DynamicTextDisplay and DynamicGraphicDisplay to know what size
pixel buffer to allocate.

Please take a look at `CoreModules/elements/base_element.hh` for the complete list of types and their fields.


### Elements array

The order of the elements in the Elements array determines the ID of each param, jack, and light.
The first param has ID 0, the second param has ID 1, etc...

To make this easy, the ElementCount namespace provides several ways to get the index of an entry in the Elements array.

TODO: Finish this


## Registering a Module:

Once you have a custom class that derives from `CoreProcessor`, and
an Info class, you need to register your module using the `register_module`
function.
There are several overloads for `register_module` function, you can read details
in [metamodule-core-interface/CoreModules/register_module.hh].

We'll discuss one common and easy to use overload here:

```c++

namespace MetaModule {

template <typename ModuleT, ModuleInfoT>
bool register_module(std::string_view brand_name); 

```

Typically you will call this from your init() function. 
Simply fill in your module and brand info like this:

```c++
void init() {
    using namespace MetaModule;

    register_module<MyModule, MyModuleInfo>("MyBrand");
    register_module<MyModule2, MyModule2Info>("MyBrand");
    //...
}
```

However, you also can call it during static global initialization, 
say for instance in the module class itself:

```c++
struct MyModule {

    static inline bool mymodule_ok = register_module<MyModule, MyModuleInfo>("MyBrand");
```

Make sure that your class `MyModuleInfo` has a member variable for the slug and png_filename like this:

```c++
struct MyModuleInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"MyModule"};
    static constexpr std::string_view png_filename{"MyBrand/faceplates/my-module-light.png"};
```

