# How plugins work

Understanding how a plugin is loaded should help make the plugin requirements clear.

- Plugins are an `.mmplugin` file. When you build a plugin, you first compile a
  binary object which is essentially your modules compiled for the MetaModule
  platform's processor. Then, the SDK scripts will combine that binary with the
  images and fonts (in the assets/ dir) and some metadata files into an .mmplugin file.
  The way these things are all combined is with the `tar` program. The SDK
  build script does all this automatically for you.

- When the firmware loads a plugin, first it untars the .mmplugin file. All of
  the image and font files (*.png and *.bin) are copied to the internal RAM
  drive into a directory with the plugin's name. The metadata files are processed
  so it knows the plugin slug, display name, etc.

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

See [docs/module-registry.md] for an example.


## ModuleInfo and Elements

The ModuleInfo defines all other aspects of a module besides what's in CoreProcessor.
Roughly speaking, the CoreProcessor class defines the audio/DSP aspects, and the ModuleInfo
class defines the UI aspects.

See [Elements docs](elements.md) for more information about creating the elements for your module.


