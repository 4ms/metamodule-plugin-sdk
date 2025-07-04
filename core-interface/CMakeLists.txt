project(metamodule-core-interface)

add_library(metamodule-core-interface INTERFACE)
add_library(metamodule::core-interface ALIAS metamodule-core-interface)

target_include_directories(metamodule-core-interface INTERFACE .)
target_include_directories(metamodule-core-interface INTERFACE ./filesystem)
