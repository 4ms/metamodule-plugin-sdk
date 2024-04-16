set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")

# chip arch
# all further targets needs to be built and linked with those options
include(${CMAKE_CURRENT_LIST_DIR}/cmake/arch_mp15xa7.cmake)
link_libraries(arch_mp15x_a7)

# Add plugin SDK
add_subdirectory(${CMAKE_CURRENT_LIST_DIR})

# PLUGIN is the name of the cmake target
function(create_plugin PLUGIN)

    set_property(TARGET ${PLUGIN} PROPERTY CXX_STANDARD 20)

    target_link_libraries(${PLUGIN} PRIVATE metamodule-sdk)

	set(LFLAGS
        -shared
        -Wl,-Map,plugin.map,--cref
        -Wl,--gc-sections
        -nostartfiles 
        -nostdlib
    )

	# Link objects into a shared library (CMake won't do it for us)
    add_custom_command(
		OUTPUT ${PLUGIN}-debug.so
		DEPENDS ${PLUGIN}
		COMMAND ${CMAKE_CXX_COMPILER} ${LFLAGS} -o ${PLUGIN}-debug.so
				$<TARGET_OBJECTS:${PLUGIN}> $<TARGET_OBJECTS:metamodule-sdk>
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

    add_custom_target(plugin ALL DEPENDS ${PLUGIN}.so)

	# TODO: generate plugin directory structure
	# TODO: copy artwork files (given a dir) to the plugin dir
	# TODO: ?? target to convert a dir of SVGs to PNGs?

endfunction()


