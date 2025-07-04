# Native Plugins

A "Native Plugin" is one that's not based on an existing VCV Rack plugin.

For full examples, see NativeExample and Airwindows in the [metamodule
plugins examples repo](https://github.com/4ms/metamodule-plugin-examples)

## How plugins work

Understanding how a plugin is loaded should help make the plugin requirements clear:

- First, the plugin load untars the .mmplugin file. All of the image and font
  files (*.png and *.bin) are copied to the internal RAM drive into a directory
  with the plugin's name.

- Next, the `.so` file is parsed (as an elf file) and all dynamic relocations
  are performed. Without going into detail, relocations are how the plugin is
  able to call functions defined in the firmware.
  If there are unresolved symbols (that is, if a plugin makes function calls
  that are not present in the firmware), they are reported and it aborts.

- Next, the global static constructors are called. This initializes global
  variables and static class members.

- Finally, the `init()` function is called. Both `init()` and
  `init(rack::Plugin::plugin *)` are searched, with the latter taking
  precedence. The plugin must define this function. Typically, this function
  is used to register all the modules.

## Registering a module

If you are familiar with VCV Rack plugins, you might recall that in a typical
VCV Rack plugin, modules are registered with calls to `p->addModel(modelName)`
in `init(rack::plugin *p)`. 

With native plugins you can register modules using the function
`register_module()`. This function is declared in
`core-interface/CoreModules/register_module.hh`:

```c++

template <typename ModuleT, ModuleInfoT>
bool register_module(std::string_view brand_name, 
                     std::string_view module_slug,
                     std::string_view faceplate_filename)  { /*...*/ }
```

Here is an outline of typical usage:
```c++

class MyModuleClass : CoreProcessor {
    //... 
};

struct MyModuleInfo : ModuleInfoBase {
    //...
};

void init() {
    register_module<MyModuleClass, MyModuleInfo>("MyBrand", "MyModule", "MyBrand/faceplates/module1.png");
}

```

The `MyModuleClass` template parameter is your module class, which must be derived
from `CoreProcessor`. See CoreProcessor section below.

`MyModuleInfo` template parameter defines your module's elements (knobs, jacks,
etc). See ModuleInfo section below.

The `brand_name` and `module_slug` parameters are the plugin name and the module name. These 
must match what's present in VCV Rack if you want your users to be able to create
patches in VCV Rack using your modules.

`faceplate_filename` is the full path to the faceplate PNG file.

The `register_module` function returns a bool to indicate if the module was
registered ok. You normally don't need to check the return value.

## CoreProcessor

The `CoreProcessor` class is a virtual base class which is the base of all
MetaModule modules:

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

Your module class must derive from this, and should implement as many of the
virtual functions as it needs to. At minimum, it has to implement update(),
set_samplerate(), set_param(), set_input(), and get_output().

TODO: discussion on what each function does.

### Customizing the module's factory function

When registering the module, you can use the `register_module()` function
template discussed above. But if you need more control over how the module
object is created, you can pass a function that returns a std::unique_ptr to
your module class like this: 

```c++
// Say, for instance we need to pass `someParameter` to all instances of our
// module's constructor. We could do it like this:
register_module(
    "MyBrand", "MyModule", 
    [&]() { return std::make_unqiue<MyModuleClass>(someParameter); }, 
    info, faceplate);

// Note that you need to also define `info`, see next section:
```


## ModuleInfo

The ModuleInfo defines all other aspects of a module besides what's in CoreProcessor.
Roughly speaking, the CoreProcessor class defines the audio/DSP aspects, and the ModuleInfo
class defines the UI aspects.

There are several ways to create the ModuleInfo, depending on your preference.
In the end, both techniques do the same two things:
 - Create a list of elements (and other things) that will persist for the lifetime of the plugin.
 - Pass a "view" of these elements to the register_module() function.

### Technique 1: Creating ModuleInfoView via ModuleInfo class

This technique is to create a class that's derived from `ModuleInfoBase`.
You can see this base class in [element_info.hh](../core-interface/CoreModules/elements/element_info.hh)

Then we pass this class to `register_module` as a template parameter.

For example:

```c++
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

void init() {
    register_module<MyModuleClass, MyModuleInfo>("MyBrand", "MyModule", "MyBrand/module-fp.png");
}
```


The types in the `Elements` array such as `Knob`, `JackInput`, etc are all defined in
[core-interface/CoreModules/elements/base_element.hh](../core-interface/CoreModules/elements/base_element.hh)
Notice that the `Elements` array contains `Element` objects. The `Element` type is a `std::variant` of
all the different types of controls, jacks, buttons, etc a module can have (`Knob`, `JackInput`, `JackOutput`, etc).
You can see all the alternative types in `Element` in [elements.hh](../core-interface/CoreModules/elements/elements.hh)

//TODO: more info on Element types, making custom types.//

4ms modules use this technique, by defining custom types derived from the Element types.
This reduces all the braces needed and also repeating things like the image name.
See the [info files in the CoreModules repo](https://github.com/4ms/metamodule-core-modules/tree/main/4ms/info) for examples.


### Technique 2: Creating ModuleInfoView directly

Instead of the above method, another way is to create your ModuleInfoView at runtime.

ModuleInfoView is defined as follows:

```c++
struct ModuleInfoView {
	std::string_view description{""};
	uint32_t width_hp = 0;
	std::span<const Element> elements;
	std::span<const ElementCount::Indices> indices;
    ///other stuff, we're ignoring for this example
};

```

Notice this contains a std::span to some Elements, and a std::span to some Indices. You have
to create arrays or vectors manually and then point the fields of the ModuleInfoView to
these arrays or vectors. The arrays or vectors you make need to have a lifetime that 
is the entire duration of the plugin (that is, static or global data).

Here's an example from the SimpleGain module of the NativeExample project:

```c++
void init_simple_gain() {
  // Notice these are static! They must live for the entire plugin's lifetime
  static std::array<MetaModule::Element, 4> elements;
  static std::array<ElementCount::Indices, 4> indices;

  MetaModule::Knob gain;
  gain.x_mm = 20;
  gain.y_mm = 20;
  gain.image = "NativeExample/components/knob.png";
  gain.short_name = "Gain";
  elements[0] = gain;
  indices[0] = {.param_idx = GainKnobID};

  MetaModule::JackInput injack;
  injack.x_mm = 20;
  injack.y_mm = 60;
  injack.image = "NativeExample/components/jack.png";
  injack.short_name = "Input";
  elements[1] = injack;
  indices[1] = {.input_idx = InputJackID};

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

