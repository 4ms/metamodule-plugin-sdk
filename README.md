# MetaModule Plugin SDK

The MetaModule Plugin SDK is used for building a plugin to run on MetaModule hardware.
Often this uses an existing Rack plugin, rebuilding it as a MetaModule plugin. However, you 
may also use this SDK to create a native plugin that's not based on an existing Rack plugin.

For example projects using this SDK, see [metamodule-plugin-examples](https://github.com/4ms/metamodule-plugin-examples)

## Requirements

  - cmake v3.24 or later
  - ninja (not required if you configure cmake to use a different generator) 
  - arm-none-eabi-gcc toolchain 12.2 or 12.3
  - python 3.6 or later

## More Reading:
 - [Text Screens/Displays](docs/text-displays.md)
 - [Native Plugins](docs/native-plugin.md)
 - [Licensing and Permissions](docs/licensing_permissions.md)
 - [Releasing a plugin](docs/release.md)
 - [Plugin file format](docs/plugin-file-format.md)
 - [API reference](docs/api.md)
 - [Tips](docs/tips.md)

## Basic Example for Converting a Rack Plugin

1. Install this repo and create a new project folder (not in the same dir as this repo):

```bash
git clone https://github.com/4ms/metamodule-plugin-sdk --recursive
mkdir MyPlugin
cd MyPlugin
```

2. Create a CMakeLists.txt file in the project folder. The example below references an existing directory
at `../../rack-plugins/MyPlugin` that contains `src/plugin.cpp` and
`src/MyModule1.cpp` and `src/MyModule2.cpp`.

`MyPlugin/CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.19)
include(../metamodule-plugin-sdk/plugin.cmake)

project(MyPlugin VERSION 0.1 DESCRIPTION "My Plugin for MetaModule" LANGUAGES C CXX ASM)

add_library(MyPlugin STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR ../../rack-plugins/MyPlugin)

target_sources(MyPlugin PRIVATE
    ${SOURCE_DIR}/src/plugin.cpp                    # Note: this file contains init(rack::Plugin*)
    ${SOURCE_DIR}/src/MyModule1.cpp
    ${SOURCE_DIR}/src/MyModule2.cpp
)

# Add includes and compile options for source repo
target_include_directories(MyPlugin PRIVATE ${SOURCE_DIR}/src)

# Call this to link and create the plugin file
create_plugin(
    SOURCE_LIB      MyPlugin                                          # The cmake target name (defined in add_target)
    PLUGIN_NAME     MyPlugin                                          # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${SOURCE_DIR}/plugin.json                         # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets                  # Path to the assets/ dir containing the PNGs
    DESTINATION     ${CMAKE_CURRENT_LIST_DIR}/../metamodule-plugins   # Path to where you want the plugin file output
)
```

Notice that you must give the path to the `plugin.json` file that the Rack-SDK requires you to have.
This is used to get the brand name and slug.

3. Create a `plugin-mm.json` file.
This contains MetaModule-specific information about the plugin and the modules it provides.

Read more about this file [here](docs/plugin-mm-json.md).

An minimal example could be this:

`MyPlugin/plugin-mm.json`:
```json
{
	"MetaModuleBrandName": "Plugin Display Name",
}

```

4. Add any graphical assets your project uses. See [Graphics Guide](docs/graphics.md).

If you are just trying to get it compiling and don't have PNGs yet, then you can just
create an empty directory.

```bash
mkdir assets
cp location/of/png/files/*.png assets/
```

5. Now build your project:

```bash
cmake -B build -G Ninja
cmake --build build
```

You must have the `arm-none-eabi-gcc` toolchain on your PATH. Or, you can specify the path to it
with the Cmake variable `TOOLCHAIN_BASE_DIR`:

```base
cmake -B build -G Ninja -DTOOLCHAIN_BASE_DIR=/path/to/arm-gnu-toolchain-12.3-arm-none-eabi/bin
```

## Format of a plugin file


A MetaModule Plugin is an .mmplugin file, which is a tarball. The plugin.cmake script
takes care of this for you, but it can be useful to untar a plugin when debugging. To
view the contents of a plugin:

```bash
tar -xf MyPlugin.mmplugin
ls MyPlugin/
```

You will probably see files like this:
```bash
MyPlugin.so            # The plugin binary shared object
SDK-1.3                # Generated by plugin.cmake (empty file)
plugin.json            # Metadata: See above
plugin-mm.json         # Metadata: See above
faceplate-module1.png  # Everything below is the contents of 
faceplate-module2.png  # the assets/ directory
components/

...
```

## Limitations

As of v1.5, plugins have the following limitation:

  - No use of stringstream, fstream, ofstream, etc. Calls to these functions must be removed.

  - C++ exceptions must be disabled. Calls to throw or try/catch must be removed.


One way to deal with these to use `#ifdef METAMODULE` around sections that must
be removed. As we add support for things like exceptions and streams, it'll be
easy to search and remove these workarounds.

To allow for these, we need to re-build our custom libstdc++/libsupc++
libraries with -fPIC as well as exceptions and streams support (HELP WANTED!).

If you are porting from a VCV Rack plugin, there are also the following limitations:

  - No dynamic drawing using nanovg (NVG draw commands). Calling these
    functions will still compile and run, but MetaModule does not call
    Widget::draw() or drawLayer(), so they'll have no effect. This typically
    means that modules with non-text screens will just have a blank screen. 
      - In order to use a text-only screen, follow [the Text Display guide](docs/text-displays.md)
      - Graphical drawing via nanovg is partially supported on the v2.0-dev
        firmware branch (no concave filled polygons or text, yet).

  - No expander modules. That is, modules cannot communicate with one another.
    Modules that use an expander will always act as if the expander is not
    present.

  - The module's `rack::ModuleWidget` is constructed once when the plugin is
    loaded, and then destructed and never again constructed. Therefore
    `rack::Widget::step()` or `draw()` is never called on the ModuleWidget
    or any children widgets. This will change with the support of graphical elements in v2.0.

The next major firmware release (v2.0) will address some of these issues:

  - v2.0-dev firmware supports graphical screens (dynamic drawing)
    `step()` and `draw()` and `drawLayer()` are called for widgets that otherwise
    aren't drawn by the MetaModule GUI engine.

  - `ModuleWidget::step()` is called for all ModuleWidgets. If there is a
    custom `draw()` or `drawLayer()` then a full-module-sized canvas will be
    created and these will be called to draw onto it.

  - v2.0-dev supports non-blocking (asynchronious) tasks, which you can use to
    make filesystem calls. It also supports POSIX and FatFS filesystem API to
    access files in the plugin .mmplugin file, or anywhere on the SD Card or
    USB drive. There is a file picker and file save dialog box so users can
    locate files or specify a path to save into.



## Licensing/Permissions 

If you are porting a plugin which you are not the original author, read this
before you get too far in the porting process: (Licensing and
Permissions)[licensing_permissions.md]



