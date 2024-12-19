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

## SDK Topics:
 - [Text Screens/Displays](docs/text-displays.md)
 - [Async Thread API](docs/async-threads.md)
 - [Native Plugins](docs/native-plugin.md)
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

2. Create a CMakeLists.txt file in the project folder. 

The example below references an existing directory at
`../../rack-plugins/MyPlugin` that contains the VCV Rack plugin source files.

`MyPlugin/CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.24)
include(../metamodule-plugin-sdk/plugin.cmake)

project(MyPlugin VERSION 0.1 DESCRIPTION "My Plugin for MetaModule" LANGUAGES C CXX ASM)

add_library(MyPlugin STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR ../../rack-plugins/MyPlugin)

target_sources(MyPlugin PRIVATE
    ${SOURCE_DIR}/src/plugin.cpp 
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

3. Create a `plugin-mm.json` file. Read more about this file [here](docs/plugin-mm-json.md).
You can just use a blank file to get started.

```bash
touch plugin-mm.json


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

You must have the `arm-none-eabi-gcc` toolchain v12 on your PATH. Or, you can
specify the path to it with the Cmake variable `TOOLCHAIN_BASE_DIR`:

```base
cmake -B build -G Ninja -DTOOLCHAIN_BASE_DIR=/path/to/arm-gnu-toolchain-12.3-arm-none-eabi/bin
```


## Limitations

In short:

- No stringstream, fstream, ofstream, iostream, etc.
- No C++ exceptions (no try/catch, no throw)
- Filesystem access via fopen/fread/etc is read-only, and limited to files loaded from the .mmplgugin file. (coming in v2.x)
- No dynamic drawing of widgets (coming in v2.x)
- No expander modules

See [limitations](docs/limitations.md) for more discussion.





