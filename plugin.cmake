cmake_minimum_required(VERSION 3.22)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")


add_subdirectory(
	${CMAKE_CURRENT_LIST_DIR}/metamodule-rack-interface 
	${CMAKE_CURRENT_BINARY_DIR}/metamodule-rack-interface
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/arch_mp15xa7.cmake)
target_link_libraries(metamodule-rack-interface PUBLIC arch_mp15x_a7)

# Usage: create_plugin(PluginTarget)
# PluginTarget is the name of the cmake target
macro(create_plugin)

	set(PLUGIN ${ARGV0})

    target_link_libraries(${PLUGIN} PRIVATE metamodule-rack-interface)

	set(LFLAGS
        -shared
        ${ARCH_MP15x_A7_FLAGS}
        -Wl,-Map,plugin.map,--cref
        -Wl,--gc-sections
        -nostartfiles -nostdlib
    )

	# Link objects into a shared library (CMake won't do it for us)
    add_custom_command(
		OUTPUT ${PLUGIN}-debug.so
		DEPENDS ${PLUGIN}
		COMMAND ${CMAKE_CXX_COMPILER} ${LFLAGS} -o ${PLUGIN}-debug.so
				$<TARGET_OBJECTS:${PLUGIN}> $<TARGET_OBJECTS:metamodule-rack-interface>
		COMMAND_EXPAND_LISTS
		VERBATIM USES_TERMINAL
    )

	# Strip symbols to create a smaller plugin file
    add_custom_command(
        OUTPUT ${PLUGIN}.so
        DEPENDS ${PLUGIN}-debug.so
        COMMAND ${CMAKE_STRIP} -g -v -o ${PLUGIN}.so ${PLUGIN}-debug.so
		COMMAND ls -l ${PLUGIN}.so
        VERBATIM USES_TERMINAL
    )

	# TODO: generate plugin directory structure
	# TODO: copy artwork files (given a dir) to the plugin dir
	# TODO: ?? target to convert a dir of SVGs to PNGs?

    add_custom_target(plugin ALL DEPENDS ${PLUGIN}.so)

endmacro()


