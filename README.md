# MetaModule Plugin SDK

The MetaModule Plugin SDK is used for building a plugin to run on MetaModule hardware.
Often this uses an existing Rack plugin, rebuilding it as a MetaModule plugin. However, you 
may also use this SDK to create a native plugin that's not based on an existing Rack plugin.

For example projects using this SDK, see [metamodule-plugin-examples](https://github.com/4ms/metamodule-plugin-examples)

## Requirements

  - cmake v3.22 or later
  - ninja (not required if you configure cmake to use a different generator) 
  - arm-none-eabi-gcc toolchain 12.2 or 12.3
  - python 3.6 or later
  - Optional (recommended): jq (for validating plugin-mm.json file)

## API Reference:
 - [CoreProcessor class](docs/coreprocessor.md)
 - [Elements](docs/elements.md)
 - [Registering a module](docs/module-registry.md)
 - [Text Screens/Displays](docs/text-displays.md)
 - [Graphical Screens/Displays](docs/graphic-displays.md)
 - [System API](docs/system-api.md)
     - Audio Settings
     - File browser
     - Filesystem calls
     - Notifications
     - Patch files
     - Memory
     - Random
     - Time
 - [Async Thread API](docs/async-threads.md)
 - [DSP helper classes](docs/dsp.md)
 - [Graphics helper classes](docs/graphics-helpers.md)
 - [Wav File Streaming class](docs/wav-file-stream.md)

 ## General topics:
 - [How Plugins Work](docs/how-plugins-work.md)
 - [Licensing and Permissions](docs/licensing_permissions.md)
 - [Releasing a plugin](docs/release.md)
 - [Plugin file format](docs/plugin-file-format.md)
 - [Tips](docs/tips.md)

## Basic Example for Converting a Rack Plugin

1. Install this repo and create a new project folder (not in the same dir as this repo):

```bash
git clone https://github.com/4ms/metamodule-plugin-sdk --recursive
mkdir MyPlugin
cd MyPlugin
```

2. Create a CMakeLists.txt file in the project folder. 

The example below references an existing directory at
`../../rack-plugins/MyPlugin` that contains the VCV Rack plugin source files.

`MyPlugin/CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.22)

# Set the path to the Plugin SDK.
# In a real plugin, you want to use a cmake variable instead of hard-setting the path, see below...
include(../metamodule-plugin-sdk/plugin.cmake)

project(MyPlugin VERSION 0.1 DESCRIPTION "My Plugin for MetaModule" LANGUAGES C CXX ASM)

add_library(MyPlugin STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR ../../rack-plugins/MyPlugin)

target_sources(MyPlugin PRIVATE
    ${SOURCE_DIR}/src/plugin.cpp 
    ${SOURCE_DIR}/src/MyModule1.cpp
    ${SOURCE_DIR}/src/MyModule2.cpp)

# Add includes and compile options for source repo
target_include_directories(MyPlugin PRIVATE ${SOURCE_DIR}/src)

# Call this to link and create the plugin file
create_plugin(
    SOURCE_LIB      MyPlugin                                        # The cmake target name (defined in add_target)
    PLUGIN_NAME     MyPlugin                                        # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${SOURCE_DIR}/plugin.json                       # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets                # Path to the assets/ dir containing the PNGs
    DESTINATION     ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins    # Path to where you want the plugin file output
)
```

Notice that you must give the path to the `plugin.json` file that the Rack-SDK requires.
This file is read by the MetaModule in order to get the brand name and slug.

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

4. Add any graphical assets (svgs must be converted to png) and fonts (ttf) your project uses. See [Graphics Guide](docs/graphics.md).

If you are just trying to get it compiling and don't have PNGs yet, then you can just
create an empty directory.

```bash
mkdir assets
cp location/of/png/files/*.png assets/
```

5. Now build your project:

```bash
cmake --fresh -B build -G Ninja
cmake --build build
```

You must have the `arm-none-eabi-gcc` toolchain v12 on your PATH. Or, you can
specify the path to it with the Cmake variable `TOOLCHAIN_BASE_DIR`:

```bash
cmake --fresh -B build -G Ninja -DTOOLCHAIN_BASE_DIR=/path/to/arm-gnu-toolchain-12.3-arm-none-eabi/bin
```

## Path to the SDK

In the example CMakeLists.txt file above, the path the SDK was hard-set. While this will work, it's not a good idea. 
If you ever want to run your plugin in the simulator, it will fail since the simulator needs to specify it's own "simulator SDK"
in place of the normal SDK.

So, the best practice is to copy/paste some boilerplate into your CMakeLists file.
Replace the `include(../metamodule-plugin-sdk/plugin.cmake)` line in the example code with this:

```cmake

if(NOT "${METAMODULE_SDK_DIR}" STREQUAL "")
	message("METAMODULE_SDK_DIR set by CMake variable ${METAMODULE_SDK_DIR}")
elseif (DEFINED ENV{METAMODULE_SDK_DIR})
    set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
	message("METAMODULE_SDK_DIR set by environment variable ${METAMODULE_SDK_DIR}")
else()
    set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../metamodule-plugin-sdk")
	message("METAMODULE_SDK_DIR set to default: ${METAMODULE_SDK_DIR}")
endif()

include(${METAMODULE_SDK_DIR}/plugin.cmake)

```

What this does is two-fold:

1) Lets you specify a path to the SDK either using a cmake variable, or using
an environment variable. If you specify neither, it will default to the SDK
being in the parent dir, just like the first example code.

2) Outputs a message telling you where the SDK its using is found, and what
made it use that path. This is useful for debugging build issues.

To specify the path, you can do it a different ways.

Using an exported environment variable:

```bash
# Put this in your .bashrc or .zshrc:
export METAMODULE_SDK_DIR=/path/to/metamodule-plugin-sdk

# Configure normally and it will find the SDK
cmake --fresh -B build -G Ninja

# Build normally
cmake --build build
```

Using a cmake variable:

```bash
# Specify the path once when you first configure the project
cmake --fresh -B build -G Ninja -DMETAMODULE_SDK_DIR=/path/to/metamodule-plugin-sdk

# Build normally:
cmake --build build
```

You also can doing neither of the above and just make sure the SDK is in the parent dir.


## Limitations

In short:

- No stringstream, fstream, ofstream, iostream, etc.
- No C++ exceptions (no try/catch, no throw)
- No expander modules
- No support for loading or rendering SVGs (via nanoSvg)
- Param, Jack, and Light widgets are drawn with the MetaModule engine, not with nanovg. Children of these widgets are ignored.

See [limitations](docs/limitations.md) for more discussion.

## Licensing/Permissions 

If you are porting a plugin which you are not the original author, read this
before you get too far in the porting process: (Licensing and
Permissions)[licensing_permissions.md]



