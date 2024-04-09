# MetaModule Plugin SDK

WIP

Used for building an existing Rack plugin as a MetaModule plugin.

To use:

```
git clone https://github.com/4ms/metamodule-plugin-sdk.git --recursive
mkdir MyPlugin
cd MyPlugin

cat << EOF > CMakeLists.txt

cmake_minimum_required(VERSION 3.19)
include(../metamodule-plugin-sdk/plugin.cmake)

project(MyPlugin VERSION 0.1 DESCRIPTION "My Plugin for MetaModule" LANGUAGES C CXX )

add_library(MyPlugin STATIC)

# Add sources from the Rack source repo
set(SOURCEDIR ../../rack-plugins/MyPlugin)

target_sources(MyPlugin PUBLIC
    ${SOURCEDIR}/src/plugin.cpp #contains init(rack::Plugin*)
    ${SOURCEDIR}/src/MyModule1.cpp
    ${SOURCEDIR}/src/MyModule2.cpp
)

# Add includes and compile options for source repo
target_include_directories(MyPlugin PRIVATE ${SOURCEDIR}/src)

# Call this to link and create the plugin file
create_plugin("MyPlugin")
EOF

cmake -B build -GNinja
cmake --build build

```

