# MetaModule Plugin SDK

Used for building an existing Rack plugin as a MetaModule plugin.

For example projects using this SDK, see [metamodule-plugin-examples](https://github.com/4ms/metamodule-plugin-examples)

## Basic Usage

```bash
git clone https://github.com/4ms/metamodule-plugin-sdk.git --recursive
mkdir MyPlugin
cd MyPlugin

cat << EOF > CMakeLists.txt

cmake_minimum_required(VERSION 3.19)
include(../metamodule-plugin-sdk/plugin.cmake)

project(MyPlugin VERSION 0.1 DESCRIPTION "My Plugin for MetaModule" LANGUAGES C CXX ASM)

add_library(MyPlugin STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR ../../rack-plugins/MyPlugin)

target_sources(MyPlugin PRIVATE
    ${SOURCE_DIR}/src/plugin.cpp #contains init(rack::Plugin*)
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
EOF

cat << EOF > plugin-mm.json
{
	"MetaModulePluginMaintainer": "My Name",
	"MetaModulePluginMaintainerEmail": "",
	"MetaModulePluginMaintainerUrl": "",
	"MetaModuleDescription": "",
    "MetaModuleIncludedModules": [ "Module1", "Module2" ]
}
EOF

cmake -B build -G Ninja
cmake --build build

```

Plugins require a `plugin.json` file in the same format as the VCV Rack
`plugin.json`. Some additional MetaModule-specific information must be supplied
in the `plugin-mm.json` file. This information is used when cataloging the
plugin. The fields in this file refer to the MetaModule plugin, which may be
different than the maintainer of the main repo (e.g. different maintainers,
different list of modules).

Currently this metadata is only used to display and catalog plugins on the website, 
but future firmware will parse the json files, so make sure the information is accurate.


## Requirements

  - cmake v3.24 or later
  - ninja (not required if you configure cmake to use a different generator) 
  - arm-none-eabi-gcc toolchain 12.2 or 12.3
  - python 3.6 or later

## Limitations

As of now, there are the following limitations:

  - No dynamic drawing using nanovg (NVG draw commands). Calling these
    functions will still compile and run, but MetaModule does not call
    Widget::draw() or drawLayer(), so they'll have no effect. This typically
    means that modules with screens will just have a blank screen. There is support
    for dynamically updating text-only screens, but it requires adding
    a function to the rack::Module class.

  - No expander modules. That is, modules cannot communicate with one another.
    Modules that use an expander will always act as if the expander is not
    present.

  - No filesystem access. Filesystem calls must be removed/#ifdef/commented out.

  - No use of stringstream, fstream, ofstream, etc. Calls to these functions must be removed.

  - C++ exceptions must be disabled. Calls to throw or try/catch must be removed.


The easiest way to work around any of these is to create a branch or fork of
the original code and comment or #ifdef out the offending code. 


We plan to address these:

  - We already have limited support for text-only screens. We will document this soon!

  - To support graphical screens (dynamic drawing) we plan to implement an
    adaptor to go from nanovg to our native GUI engine, and call draw() on all
    visible widgets in the ModuleView (refresh rate will be limited).

  - Create a non-blocking (asynchronious) filesystem API to access files in the
    plugin dir (assuming the user hasn't ejected the USB drive or SD card). There is 
    a proof-of-concept branch for this working.

  - Re-build our custom libstdc++/libsupc++ libraries with exceptions and streams
    support (HELP WANTED!).


## Graphics

VCV Rack uses SVG files for graphical assets, but MetaModule uses PNGs. So, we
need to convert all SVGs to PNGs. Typically all SVGs are kept in a `res/`
directory for VCV Rack plugins. For MetaModule plugins, the `res/` dir is
omitted, but otherwise the directory structure and file base names are kept the
same. Before building your plugin, convert all the SVGs to PNGs and put them
into the `assets/` dir in your plugin folder. This `assets/` dir is refered to
by CMakeLists in the call to `create_plugin()`, so if you want to use a
different directory, just change it there. The Cmake script will simply copy
the contents of the `assets/` dir to the plugin dir.

There is a helper script that can convert a directory of SVGs to PNGs:
`scripts/SvgToPng.py`. Running `scripts/SvgToPng.py -h` will display the help
with available options. The script requires that you have inkscape installed
and the `inkscape` executable on your PATH. Version 1.2.2 has been tested to
work.

You can use it to convert directories, one at a time (it does not recursively
scan sub-dirs):

```bash
cd MyPlugin

mkdir assets 
mkdir assets/panels 
mkdir assets/components

# Convert res/panels/*.svg ==>> assets/panels/*.png: 
../scripts/SvgToPng.py --input ../path/to/rack_plugins/MyPlugin/res/panels/ --output assets/panels

# Convert res/components/*.svg ==>> assets/components/*.png:
../scripts/SvgToPng.py --input ../path/to/rack_plugins/MyPlugin/res/components/ --output assets/components
```

The script is not sophisticated: it will try to figure out the SVG's DPI but
may get it wrong. If you need to, use the `--height=240` option when
converting faceplates to force them to be 240px. You can also specify this option
for component artwork if the script is making them the wrong size by default.

If you want to disable transparency, then add the `--white` option.

Sometimes, you may wish to not use the script at all and just generate your own
PNG files that are better suited for the low-resolution screen of the MetaModule.
This will produce the best results, but of course is time-consuming.

Certain special effects like shadows, subtle gradients, thin lines will not look
good on the MetaModule screen, so keep that in mind as you convert your artwork.

The screen is 240px x 320px, about 144 ppi. We use 16-bit color (RGB565).
Modules are displayed 240px high when viewed full-screen and 180px high when zoomed-out.
We may add future options to zoom out to 120px when zoomed-out.
