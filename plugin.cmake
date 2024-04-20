set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ccache.cmake)

# Whether to compile with static libc and libm
set(METAMODULE_PLUGIN_STATIC_LIBC 0)

# Set the chip architecture
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

	target_link_libraries(${LIB_NAME} PRIVATE metamodule-sdk)

	set(LFLAGS
        -shared
        -Wl,-Map,plugin.map,--cref
        -Wl,--gc-sections
        -nostartfiles 
        -nostdlib
        ${ARCH_MP15x_A7_FLAGS}
    )

    if (METAMODULE_PLUGIN_STATIC_LIBC)
        set(LINK_LIBS_DIR ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/metamodule-plugin-libc/lib)
        find_library(LIBCLIB "pluginc" PATHS ${LINK_LIBS_DIR} REQUIRED)
        find_library(LIBMLIB "pluginm" PATHS ${LINK_LIBS_DIR} REQUIRED)
        set(LINK_STATIC_LIBC
            -lpluginc
            -lpluginm
        )
    endif()

    get_target_property(LIBC_BIN_DIR metamodule-plugin-libc BINARY_DIR)
    find_library(LIBC_BIN_DIR "metamodule-plugin-libc" PATHS ${LIBC_BIN_DIR} REQUIRED)

	# Link objects into a shared library (CMake won't do it for us)
    add_custom_command(
		OUTPUT ${PLUGIN_FILE_FULL}
		DEPENDS ${LIB_NAME}
		COMMAND ${CMAKE_CXX_COMPILER} ${LFLAGS} -o ${PLUGIN_FILE_FULL}
				$<TARGET_OBJECTS:${LIB_NAME}>  #FIXME: libraries linked to LIB_NAME target will not be included
                -lgcc
                -L${LIBC_BIN_DIR} -lmetamodule-plugin-libc #FIXME: silently fails if this lib is not found
		COMMAND_EXPAND_LISTS
		VERBATIM USES_TERMINAL
    )

	# Strip symbols to create a smaller plugin file
    add_custom_command(
        OUTPUT ${PLUGIN_FILE}
        DEPENDS ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_STRIP} -g -v -o ${PLUGIN_FILE} ${PLUGIN_FILE_FULL}
		COMMAND ${CMAKE_SIZE_UTIL} ${PLUGIN_FILE}
        VERBATIM USES_TERMINAL
    )
    add_custom_target(plugin ALL DEPENDS ${PLUGIN_FILE})

    # Verify symbols will be resolved
    set(FIRMWARE_SYMTAB_PATH ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/api-symbols.txt)
    add_custom_command(
        TARGET plugin
        POST_BUILD
        COMMAND scripts/check_syms.py 
            --plugin ${PLUGIN_FILE}
            --api ${FIRMWARE_SYMTAB_PATH}
            # -v
        WORKING_DIRECTORY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}
        VERBATIM USES_TERMINAL
    )

    # Helpful outputs for debugging plugin elf file:
    add_custom_command(
        OUTPUT ${PLUGIN_FILE_FULL}.nm
        DEPENDS ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_NM} -CA ${PLUGIN_FILE_FULL} > ${PLUGIN_FILE_FULL}.nm
    )
    add_custom_command(
        OUTPUT ${PLUGIN_FILE_FULL}.readelf
        DEPENDS ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_READELF} --demangle=auto -a -W ${PLUGIN_FILE_FULL} > ${PLUGIN_FILE_FULL}.readelf
    )
    add_custom_target(debugelf ALL DEPENDS 
        ${PLUGIN_FILE_FULL}.readelf
        ${PLUGIN_FILE_FULL}.nm
    )

    # Dissassembly can be take a long time/space, so don't always run:
    add_custom_command(
        OUTPUT ${PLUGIN_FILE_FULL}.diss
        DEPENDS ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_OBJDUMP} -CDz --source ${PLUGIN_FILE_FULL} > ${PLUGIN_FILE_FULL}.diss
    )
    add_custom_target(debugdiss DEPENDS 
        ${PLUGIN_FILE_FULL}.diss
    )

	# TODO: ?? target to convert a dir of SVGs to PNGs?

endfunction()

