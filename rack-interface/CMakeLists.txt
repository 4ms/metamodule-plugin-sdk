add_library(rack-interface INTERFACE)
add_library(metamodule::rack-interface ALIAS rack-interface)

target_include_directories(rack-interface SYSTEM INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/dep/include
)

set_property(TARGET rack-interface PROPERTY CXX_STANDARD 20)
