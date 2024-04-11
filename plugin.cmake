set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")

# metamodule-rack-interface: library to interface with RackSDK adaptor
include(${CMAKE_CURRENT_LIST_DIR}/metamodule-rack-interface/interface.cmake)

# chip arch
include(${CMAKE_CURRENT_LIST_DIR}/cmake/arch_mp15xa7.cmake)
target_link_libraries(metamodule-rack-interface INTERFACE arch_mp15x_a7)

# cpputil
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/cpputil cpputil)

# Plugin library
add_library(metamodule-sdk STATIC ${CMAKE_CURRENT_LIST_DIR}/libc_stub.c)
target_link_libraries(metamodule-sdk PUBLIC metamodule-rack-interface cpputil)
target_compile_options(metamodule-sdk PUBLIC
	-shared
	-fPIC
	-nostartfiles
	-nostdlib
)


# PLUGIN is the name of the cmake target
function(create_plugin PLUGIN)

    set_property(TARGET ${PLUGIN} PROPERTY CXX_STANDARD 20)

    target_include_directories(${PLUGIN} PUBLIC 
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}
        ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/metamodule-core-interface
    )

    target_link_libraries(${PLUGIN} PRIVATE metamodule-sdk)

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


