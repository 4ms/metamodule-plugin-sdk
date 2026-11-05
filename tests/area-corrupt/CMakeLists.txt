cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED METAMODULE_SDK_DIR)
	set(METAMODULE_SDK_DIR "../../")
endif()
include(${METAMODULE_SDK_DIR}/plugin.cmake)

project(AsyncChurnTestPlugin VERSION 0.1 LANGUAGES C CXX ASM)

add_library(AsyncChurnTest STATIC main.cc)
set_property(TARGET AsyncChurnTest PROPERTY CXX_STANDARD 20)

create_plugin(
	SOURCE_LIB AsyncChurnTest
	PLUGIN_NAME AsyncChurnTest
	PLUGIN_JSON ${CMAKE_CURRENT_LIST_DIR}/plugin.json
	SOURCE_ASSETS ${CMAKE_CURRENT_LIST_DIR}/assets
	DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/plugin
)
