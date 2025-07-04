project(metamodule-core-interface)

add_library(core-interface INTERFACE)
add_library(metamodule::core-interface ALIAS core-interface)

target_include_directories(core-interface INTERFACE .)
target_include_directories(core-interface INTERFACE ./filesystem)
