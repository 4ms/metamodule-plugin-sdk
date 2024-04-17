set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")

# chip arch
# all further targets needs to be built and linked with those options
include(${CMAKE_CURRENT_LIST_DIR}/cmake/arch_mp15xa7.cmake)
link_libraries(arch_mp15x_a7)

# Add plugin SDK
add_subdirectory(${CMAKE_CURRENT_LIST_DIR})

# Function to create a ready to use plugin from a static library
function(create_plugin)

    ################

    set(oneValueArgs SOURCE_LIB SOURCE_ASSETS DESTINATION)
    cmake_parse_arguments(PLUGIN_OPTIONS "" "${oneValueArgs}" "" ${ARGN} )

    # TODO: Add more checking and validation for arguments

    set(LIB_NAME ${PLUGIN_OPTIONS_SOURCE_LIB})

    set(PLUGIN_FILE_FULL ${LIB_NAME}-debug.so)
    cmake_path(APPEND PLUGIN_FILE ${PLUGIN_OPTIONS_DESTINATION} ${LIB_NAME}.so)

    file(MAKE_DIRECTORY ${PLUGIN_OPTIONS_DESTINATION})

    if (PLUGIN_OPTIONS_SOURCE_ASSETS)
        file(COPY ${PLUGIN_OPTIONS_SOURCE_ASSETS}/ DESTINATION ${PLUGIN_OPTIONS_DESTINATION})
    endif()

    ###############

	# FIXME: why won't the sources compile if we link a STATIC or OBJECT library here, even with WHOLE_ARCHIVE?
	# target_link_libraries(${LIB_NAME} PUBLIC "$<LINK_LIBRARY:WHOLE_ARCHIVE,metamodule-plugin-libc>")
	# target_link_libraries(${LIB_NAME} PRIVATE metamodule-sdk)
	#
	# Only this works (directly putting sources into the plugin target, and doing add_subdirectory here, not in CMakeLists.txt)
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/metamodule-plugin-sdk/metamodule-plugin-libc metamodule-plugin-libc)
	target_sources(${LIB_NAME} PRIVATE ${metamodule-plugin-libc-sources})
	target_link_libraries(${LIB_NAME} PRIVATE metamodule-sdk)

	set(LFLAGS
        -shared
        -Wl,-Map,plugin.map,--cref
        -Wl,--gc-sections
        -nostartfiles 
        -nostdlib
    )

	# Link objects into a shared library (CMake won't do it for us)
    add_custom_command(
		OUTPUT ${PLUGIN_FILE_FULL}
		DEPENDS ${LIB_NAME}
		COMMAND ${CMAKE_CXX_COMPILER} ${LFLAGS} -o ${PLUGIN_FILE_FULL}
				$<TARGET_OBJECTS:${LIB_NAME}> $<TARGET_OBJECTS:metamodule-sdk>
		COMMAND_EXPAND_LISTS
		VERBATIM USES_TERMINAL
    )

	# Strip symbols to create a smaller plugin file
    add_custom_command(
        OUTPUT ${PLUGIN_FILE}
        DEPENDS ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_STRIP} -g -v -o ${PLUGIN_FILE} ${PLUGIN_FILE_FULL}
		COMMAND ls -l ${PLUGIN_FILE}
        VERBATIM USES_TERMINAL
    )

    add_custom_target(plugin ALL DEPENDS ${PLUGIN_FILE})

	# TODO: ?? target to convert a dir of SVGs to PNGs?

endfunction()


