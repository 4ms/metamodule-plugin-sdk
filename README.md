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
 - [Tips](docs/tips.md)

## Basic Example for Converting a Rack Plugin

Install this repo and create a new project folder:

```bash
git clone https://github.com/4ms/metamodule-plugin-sdk --recursive
mkdir MyPlugin
cd MyPlugin
```

Create a CMakeLists.txt file in the project folder. The example below references an existing directory
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

Create a `plugin-mm.json` file.
This contains MetaModule-specific information used when cataloging the plugin.
The fields in this file refer to the MetaModule plugin, which may be different
than the maintainer of the main repo (e.g. different maintainers, different
list of modules).

Currently this metadata is only used to display and catalog plugins on the website, 
but future firmware will parse the json files, so make sure the information is accurate.

If you have no intention of listing this on the MetaModule website, you can
just use a blank file for now.

`MyPlugin/plugin-mm.json`:
```json
{
	"MetaModulePluginMaintainer": "My Name",
	"MetaModulePluginMaintainerEmail": "",
	"MetaModulePluginMaintainerUrl": "",
	"MetaModuleDescription": "",
	"MetaModuleIncludedModules": [
	{
		"slug": "Module1",
		"name": "Module Number One"
	},
	{
		"slug": "Module2",
		"name": "Module Number Two"
	}
}

```

Add any graphical assets your project uses. See [Graphics Guide](docs/graphics.md).

If you are just trying to get it compiling and don't have PNGs yet, then you can just
create an empty directory.

```bash
mkdir assets
cp location/of/png/files/*.png assets/
```

Now build your project:

```bash
cmake -B build -G Ninja
cmake --build build
```

You must have the `arm-none-eabi-gcc` toolchain on your PATH. Or, you can specify the path to it
with the Cmake variable `TOOLCHAIN_BASE_DIR`:

```base
cmake -B build -G Ninja -DTOOLCHAIN_BASE_DIR=/path/to/arm-gnu-toolchain-12.3-arm-none-eabi/bin
```


## Limitations

As of now, there are the following limitations:

  - No dynamic drawing using nanovg (NVG draw commands). Calling these
    functions will still compile and run, but MetaModule does not call
    Widget::draw() or drawLayer(), so they'll have no effect. This typically
    means that modules with non-text screens will just have a blank screen. In order
    to use a text-only screen, follow [the Text Display guide](docs/text-displays.md)

  - No expander modules. That is, modules cannot communicate with one another.
    Modules that use an expander will always act as if the expander is not
    present.

  - No filesystem access. Filesystem calls must be removed/#ifdef/commented out.

  - No use of stringstream, fstream, ofstream, etc. Calls to these functions must be removed.

  - C++ exceptions must be disabled. Calls to throw or try/catch must be removed.


One way to deal with the last two limitations is to use `#ifdef METAMODULE` around sections
that must be removed. As we add support for things like exceptions and streams, it'll
be easy to search and remove these workarounds.

We plan to address these:

  - To support graphical screens (dynamic drawing) we plan to implement an
    adaptor to go from nanovg to our native GUI engine, and call draw() on all
    visible widgets in the ModuleView (refresh rate will be limited).

  - Create a non-blocking (asynchronious) filesystem API to access files in the
    plugin dir (assuming the user hasn't ejected the USB drive or SD card). There is 
    a proof-of-concept branch for this working.

  - Re-build our custom libstdc++/libsupc++ libraries with exceptions and streams
    support (HELP WANTED!).


