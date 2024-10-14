# Native Plugins

A "Native Plugin" is one that's not based on an existing VCV Rack plugin.

For examples of this see NativeExample and Airwindows in the [metamodule plugins examples repo](https://github.com/4ms/metamodule-plugin-examples)

At minimum, a plugin must define an init() function, but there is nothing else
required. 

- When a plugin is loaded, first all the assets are copied to the internal RAM
  drive into a subdirectory.

- Next, the `.so` file is parsed (as an elf file) and all dynamic relocations are performed. If there are unresolved symbols, they are reported and it
  aborts.

- Next, the global static constructors are called

- Finally, the init() function is called. Both `init()` and
  `init(rack::Plugin::plugin *)` are searched, with the latter taking
  precedence.

A plugin's job is to intialize plugin-wide data (if any) and register its
modules. This is usually done in init() but also can be done in the global
constructors.

In a typical VCV Rack plugin, modules are registered with calls to
`p->addModel(modelName)` in `init(plugin*)`. In the more general case, modules
can be registered using the function `register_module()`, declared in
`metamodule-core-interface` in `CoreModules/register_module.hh`:

```c++
using CreateModuleFunc = std::function<std::unique_ptr<CoreProcessor>()>;

bool register_module(std::string_view brand_name,
					 std::string_view typeslug,
					 CreateModuleFunc funcCreate,
					 ModuleInfoView const &info,
					 std::string_view faceplate_filename);
```

The `brand_name` and `typeslug` are the plugin name and the module name. These 
must match what's present in VCV Rack if you want your users to be able to create
patches in VCV Rack using your modules.

The full path to the faceplate is specified in the last parameter.

`ModuleInfoView const &info` is a view into the module's elements (knobs, jacks, etc).
Since only a view is passed to the MetaModule core interface, the actual data
must live statically for the entire duration of the plugin. There are various ways to
store data statically:
- VCV plugins use global `Model` variables.
- The NativeExample project uses `static` variables in the module init function.
- The Airwindows project uses a global vector which allocates on the heap.

`funcCreate` is a factory function that returns a unique_ptr to a
`CoreProcessor` module. The `CoreProcessor` class is a virtual base class which
is the base of all MetaModule modules:

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

Putting it all together, the NativeExample plugin demonstrates how it works
with a very simple module that provies gain to an audio signal (with an LED to show the gain).

The method used in `simple_gain_elements.cc` to generate all the elements
(jacks, knob, etc), is just for this toy example. For large sets of modules you
would probably want to generate the X,Y positions and various values either at
runtime (using a grid perhaps), or with scripts. For the 4ms modules, we use a
python script to parse SVGs and generate a c++ header file with a constexpr
array of elements. 

One "gotcha" is that if the strings are not backed by static storage (that is,
they are dynamically generated on the stack or heap), then you will need to
provide the strings in some sort of static container. The reason is that
Element type and module registry only contain string_views.
See the Airwindows `module_creator.cc` for an example.


