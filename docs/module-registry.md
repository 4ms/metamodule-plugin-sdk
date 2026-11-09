# Registering a Module:


Once you have a custom class that derives from `CoreProcessor`, and
an Info class, you need to register your module using the `register_module`
function.

See the docs on [CoreProcessor](./coreprocessor.md) and [Elements](./elements.md) for information
about creating those classes for your module.

There are several overloads for `register_module` function, you can read details
in [core-interface/CoreModules/register_module.hh](../core-interface/CoreModules/register_module.hh).

We'll discuss one common and easy to use overload here:

```c++

namespace MetaModule {

template <typename ModuleT, ModuleInfoT>
bool register_module(); 

```

Typically you will call this from your init() function. 
Simply fill in your module and info classes like this:

```c++
void init() {
    using namespace MetaModule;

    register_module<MyModule, MyModuleInfo>();
    register_module<MyVCO, MyVCOInfo>();
    //...
}
```

However, you also can call it during static global initialization, 
say for instance in the module class itself:

```c++
struct MyModule {

    static inline bool mymodule_ok = register_module<MyModule, MyModuleInfo>();
```

### The brand

All modules in a plugin are registered under the plugin's brand slug. This is
the `"slug"` field in `plugin.json`, or the `"MetaModuleBrandSlug"` field in
`plugin-mm.json` if that's present (see [plugin-mm.json](plugin-mm-json.md)).
The SDK's `create_plugin()` reads the brand slug from these files and defines
`METAMODULE_PLUGIN_BRAND_SLUG`, which `register_module()` uses.

Each module's slug must also be listed in the `"modules"` section of `plugin.json`.

The older `register_module()` overloads from SDK v2.4.0 and earlier take the
brand as their first argument (e.g. `register_module<MyModule,
MyModuleInfo>("MyBrand")`). These are deprecated, and using them produces a
compiler warning: if the brand doesn't match the brand slug, then patches, your
plugin's metadata, and the online plugin and module lists won't find your
modules. The MetaModule also prints a warning on the console (and on screen)
when it loads a plugin with a module registered under a different brand, or
with a module slug that's not listed in `plugin.json`.

Make sure that your class `MyModuleInfo` has a member variable for the slug and png_filename like this:

```c++
struct MyModuleInfo : ModuleInfoBase {
    static constexpr std::string_view slug{"MyModule"};
    static constexpr std::string_view png_filename{"MyBrand/faceplates/my-module-light.png"};
```


### Advanced customization of the module's factory function

When registering the module, you can use the `register_module()` function
template discussed above. But if you need more control over how the module
object is created, you can pass a function that returns a std::unique_ptr to
your module class like this: 

```c++
// Say, for instance we need to pass `someParameter` to all instances of our
// module's constructor. We could do it like this:
register_module(
    "MyModule", 
    [&]() { return std::make_unique<MyModuleClass>(someParameter); }, 
    info, faceplate);

// Note that you need to also define `info`, see next section:
```

