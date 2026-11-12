cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED METAMODULE_SDK_DIR)
	set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
endif()
include(${METAMODULE_SDK_DIR}/plugin.cmake)

project(InitFaultTestPlugin
	VERSION 0.1
	DESCRIPTION "Test plugin that faults in init()"
	LANGUAGES C CXX ASM
)

# How init() faults. Choose with: cmake -B build -G Ninja -DFAULT_TYPE=undef
set(FAULT_TYPE "data" CACHE STRING "How init() faults: data, prefetch, undef, or abort")
set_property(CACHE FAULT_TYPE PROPERTY STRINGS data prefetch undef abort)

if(NOT FAULT_TYPE MATCHES "^(data|prefetch|undef|abort)$")
	message(FATAL_ERROR "FAULT_TYPE is '${FAULT_TYPE}': it must be data, prefetch, undef, or abort")
endif()
message(STATUS "init() fault type: ${FAULT_TYPE}")

add_library(InitFaultTest STATIC src/main.cc)

target_compile_definitions(InitFaultTest PRIVATE INIT_FAULT_TYPE="${FAULT_TYPE}")

set_property(TARGET InitFaultTest PROPERTY CXX_STANDARD 20)

if(NOT DEFINED INSTALL_DIR)
	set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugin)
endif()

create_plugin(
	SOURCE_LIB InitFaultTest
	PLUGIN_NAME InitFaultTest
	PLUGIN_JSON ${CMAKE_CURRENT_LIST_DIR}/plugin.json
	SOURCE_ASSETS ${CMAKE_CURRENT_LIST_DIR}/assets
	DESTINATION ${INSTALL_DIR}
)
