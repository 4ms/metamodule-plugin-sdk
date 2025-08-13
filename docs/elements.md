# MetaModule Elements

Elements are the parts of a module that connect it to the outside world. These include jacks,
knobs, buttons, lights, etc.

All modules have an array of elements that's passed to the module registry when the module is registered.
Elements can be constructed during runtime but once they are registered they are not allowed to change.

Different elements have different fields that govern their behavior. For instance a knob has a field for
min and max angle, and a three-position switch has a field for the graphic files to be used for each position.


## BaseElement

All elements derive from BaseElement, which contains basic information such as position, size, and name.

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

`long_name`: not used currently.

`width_mm`, `height_mm`: Not always used since the image itself determines the
dimensions, but in case the image file cannot be found then these are used.
Also used for DynamicTextDisplay and DynamicGraphicDisplay to know what size
pixel buffer to allocate.

## Other element types

Each type has different fields which describe
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
    bool unused = false;
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

Please take a look at `CoreModules/elements/base_element.hh` for the complete list of types and their fields.

You may use these types (Knob, Slider, RgbLight, etc) or create your own types
that derive from one of these.

## Element variant type

Each of these element classes are combined together using `std::variant` to create the `Element` type:

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
                             AltParamChoiceLabeled
                             AltParamAction,

							 KnobSnapped>;
```


## Using the Element variant class

TODO: std::visit, std::holds_alternative, std::get_if, base_element()

## ModuleInfoBase

All the individual elements are combined together into an array (or vector, but it must have static storage duration).

Typically these will be put into a class that you create, called the `ModuleInfo` class. 
This class should derive from ModuleInfoBase.
ModuleInfoBase also contains some other useful meta data about your module,
such as the slug, the width, and the faceplate PNG name. 
See [core-interface/CoreModules/elements/element_info.hh].


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
(see [core-interface/CoreModules/elements/elements.hh] for the variant definition and 
[core-interface/CoreModules/elements/base_element.hh] for the element definitions).

The order of the elements in the Elements array determines the ID of each param, jack, and light.
The first param has ID 0, the second param has ID 1, etc...

To make this easy, the ElementCount namespace provides several ways to get the index of an entry in the Elements array.

TODO: more details on ElementCount and Indices.


## Creating ModuleInfo for your module

Create a new class that derives from `ModuleInfoBase`. Override all the fields that you need to set. Any field you don't override will
use the default value (which is specified in `ModuleInfoBase`).

```c++
#include "elements/element_info.hh"

struct MyModuleInfo : ModuleInfoBase {
    static constexpr std::string_view description{"The description of my module"};
    static constexpr uint32_t width_hp = 4;

	static constexpr std::array<Element, 4> Elements{{
		Knob{{{{{8.8f, 16.7f, Center, "Divide"}, "MyBrand/knob.png"}}, 0.5f}},
		JackInput{{{{8.8f, 76.8f, Center, "CV"}, "MyBrand/jack.png"}}},
		JackInput{{{{8.8f, 95.0f, Center, "Clk In"}, "MyBrand/jack.png"}}},
		JackOutput{{{{8.8f, 103.2f, Center, "Clk Out"}, "MyBrand/jack.png"}}},
	}};
};

```


## Generating an info class at runtime

Sometimes, it's not convenient to have the list of elements known before you compile. For example,
Airwindows has a registry or database which is dynamically read at runtime to generate the modules.
This obviously won't work to generate `static constexpr` arrays of elements.

To handle this approach, you can do something like this (also look at the [source for Airwindows](https://github.com/4ms/metamodule-plugin-examples/blob/main/Airwindows/module_creator.hh))

```c++
void init_moduleABC() {

  // Notice these are static! They must live for the entire plugin's lifetime
  static std::vector<MetaModule::Element> elements;
  static std::vector<ElementCount::Indices> indices;

  MetaModule::Knob gain;
  gain.x_mm = 20;
  gain.y_mm = 20;
  gain.image = "NativeExample/components/knob.png";
  gain.short_name = "Gain";
  elements.push_back(gain);
  indices.push_back({.param_idx = GainKnobID});

  MetaModule::JackInput injack;
  injack.x_mm = 20;
  injack.y_mm = 60;
  injack.image = "NativeExample/components/jack.png";
  injack.short_name = "Input";
  elements.push_back(injack);
  indices.push_back({.input_idx = InputJackID});

  //... etc for all elements
  //...

  // Now we create the ModuleInfoView using the Element and Indices arrays
  MetaModule::ModuleInfoView info{
      .description = "A simple gain module",
      .width_hp = 10,
      .elements = elements,
      .indices = indices,
      .bypass_routes = {}; //skip this for now
  };

  MetaModule::register_module<SimpleGain>("NativeExample", "SimpleGain", info,
                                          "NativeExample/simple_gain.png");

```


This technique just creates two arrays: indices and elements, and then points a ModuleInfoView 
object to them. 

If your Elements use any dynamically generated strings, then you will also need to give the strings
static storage duration. The Airwindows example does this by having a `std::list<std::string>`, and
all element names are placed in there:


```c++
```

