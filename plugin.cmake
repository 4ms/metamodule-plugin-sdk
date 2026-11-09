cmake_minimum_required(VERSION 3.19)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
project(MetaModulePluginSDK LANGUAGES C CXX ASM)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/version.cmake)

# Check ARM GCC toolchain version compatibility. Each supported major version
# has a matching prebuilt plugin-libc archive in plugin-libc/lib/.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.2.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.4.0")
        set(METAMODULE_LIBC_GCC_MAJOR 12)
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13.2.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.4.0")
        set(METAMODULE_LIBC_GCC_MAJOR 13)
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14.2.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.4.0")
        set(METAMODULE_LIBC_GCC_MAJOR 14)
    elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "15.2.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15.4.0")
        set(METAMODULE_LIBC_GCC_MAJOR 15)
    else()
        message(FATAL_ERROR
            " ARM GCC ${CMAKE_CXX_COMPILER_VERSION} is not supported."
            " The MetaModule Plugin SDK requires ARM GNU Toolchain 12.2/12.3, 13.2/13.3, 14.2/14.3, or 15.2/15.3.\n"
            " Download the correct version from:\n"
            "   https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads\n"
            " Specify the toolchain path with:\n"
            "   cmake -DTOOLCHAIN_BASE_DIR=/path/to/arm-toolchain-12.3/bin ...")
    endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
include(${CMAKE_CURRENT_LIST_DIR}/cmake/ccache.cmake)


# Set the chip architecture
include(${CMAKE_CURRENT_LIST_DIR}/cmake/arch_mp15xa7.cmake)
link_libraries(arch_mp15x_a7)

# Add plugin SDK
add_subdirectory(${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_BINARY_DIR}/plugin-sdk)

# Function to create a ready to use plugin from a static library
function(create_plugin)

    ################

    set(oneValueArgs SOURCE_LIB SOURCE_ASSETS DESTINATION PLUGIN_NAME PLUGIN_JSON)
    cmake_parse_arguments(PLUGIN_OPTIONS "" "${oneValueArgs}" "" ${ARGN} )

    # TODO: Add more checking and validation for arguments

    string(REPLACE "plugin.json" "presets/" PLUGIN_OPTIONS_PRESET_DIR ${PLUGIN_OPTIONS_PLUGIN_JSON})

    set(LIB_NAME ${PLUGIN_OPTIONS_SOURCE_LIB})

    if (DEFINED PLUGIN_OPTIONS_PLUGIN_NAME)
        set(PLUGIN_NAME ${PLUGIN_OPTIONS_PLUGIN_NAME})
    else()
        set(PLUGIN_NAME ${LIB_NAME})
    endif()

    # Path to final plugin tar file
    cmake_path(NORMAL_PATH PLUGIN_OPTIONS_DESTINATION OUTPUT_VARIABLE PLUGIN_DEST_DIR)
    cmake_path(APPEND PLUGIN_DEST_FILE ${PLUGIN_DEST_DIR} ${PLUGIN_NAME}.mmplugin)

    cmake_path(APPEND PLUGIN_DEST_TMP_DIR ${CMAKE_CURRENT_BINARY_DIR} ${PLUGIN_NAME})

    set(PLUGIN_FILE_FULL ${PLUGIN_NAME}-debug.so)
    cmake_path(APPEND PLUGIN_FILE_TMP ${CMAKE_CURRENT_BINARY_DIR} ${PLUGIN_NAME}.so)
    cmake_path(APPEND PLUGIN_FILE ${PLUGIN_DEST_TMP_DIR} ${PLUGIN_NAME}.so)

    # plugin.json file:
    if (DEFINED PLUGIN_OPTIONS_PLUGIN_JSON)
        set(PLUGIN_JSON_SOURCE ${PLUGIN_OPTIONS_PLUGIN_JSON})
        if (EXISTS ${PLUGIN_JSON_SOURCE})
            cmake_path(APPEND PLUGIN_JSON_DEST ${PLUGIN_DEST_TMP_DIR} plugin.json)
        else()
            message(FATAL_ERROR "plugin.json file at ${PLUGIN_JSON} not found.")
        endif()
    else()
        # TODO: search for it?
        message(FATAL_ERROR "You must provide the path to the `plugin.json` file with the PLUGIN_JSON argument of create_plugin()")
    endif()

    # plugin-mm.json file
    cmake_path(APPEND PLUGIN_MM_JSON_SOURCE ${CMAKE_CURRENT_SOURCE_DIR} plugin-mm.json)
    cmake_path(APPEND PLUGIN_MM_JSON_DEST ${PLUGIN_DEST_TMP_DIR} plugin-mm.json)
    if (NOT EXISTS ${PLUGIN_MM_JSON_SOURCE})
        message(FATAL_ERROR "plugin-mm.json must be in the root dir of the plugin: ${CMAKE_CURRENT_SOURCE_DIR}.")
    endif()

    # Validate plugin-mm.json: JSON syntax, and that each module slug it lists
    # also exists in plugin.json
    cmake_path(ABSOLUTE_PATH PLUGIN_JSON_SOURCE
        BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        NORMALIZE
        OUTPUT_VARIABLE PLUGIN_JSON_SOURCE_ABS)

    # If this fails, the plugin isn't packaged (see below). Remove the previous build's
    # .mmplugin first, so a failed build doesn't leave an old one looking current.
    # Packaging runs on every build, so a good build always recreates it.
    add_custom_target(VALIDATE_PLUGIN_MM_JSON ALL
        COMMAND ${CMAKE_COMMAND} -E rm -f ${PLUGIN_DEST_FILE}
        COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/check_plugin_mm_json.py
            --plugin-mm-json ${PLUGIN_MM_JSON_SOURCE}
            --plugin-json ${PLUGIN_JSON_SOURCE_ABS}
        VERBATIM
        USES_TERMINAL
        COMMENT "Validating plugin-mm.json"
    )

    # The plugin's brand slug: "MetaModuleBrandSlug" in plugin-mm.json if present,
    # otherwise "slug" in plugin.json (same as the firmware does when loading the plugin).
    # Every module must be registered under it: register_module<Module, Info>() uses
    # METAMODULE_PLUGIN_BRAND_SLUG for the brand.
    # Re-run cmake when either file changes, so the brand slug stays current
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${PLUGIN_JSON_SOURCE_ABS} ${PLUGIN_MM_JSON_SOURCE})

    file(READ ${PLUGIN_JSON_SOURCE_ABS} PLUGIN_JSON_CONTENTS)
    string(JSON PLUGIN_BRAND_SLUG ERROR_VARIABLE PLUGIN_JSON_SLUG_ERR GET ${PLUGIN_JSON_CONTENTS} slug)
    if (NOT PLUGIN_JSON_SLUG_ERR STREQUAL "NOTFOUND")
        set(PLUGIN_BRAND_SLUG "")
    endif()

    file(READ ${PLUGIN_MM_JSON_SOURCE} PLUGIN_MM_JSON_CONTENTS)
    string(JSON PLUGIN_MM_BRAND_SLUG ERROR_VARIABLE PLUGIN_MM_JSON_SLUG_ERR GET ${PLUGIN_MM_JSON_CONTENTS} MetaModuleBrandSlug)
    if (PLUGIN_MM_JSON_SLUG_ERR STREQUAL "NOTFOUND" AND NOT PLUGIN_MM_BRAND_SLUG STREQUAL "")
        set(PLUGIN_BRAND_SLUG ${PLUGIN_MM_BRAND_SLUG})
    endif()

    if (PLUGIN_BRAND_SLUG STREQUAL "")
        message(WARNING "${PLUGIN_NAME}: no brand slug found: plugin.json has no \"slug\" "
                        "and plugin-mm.json has no \"MetaModuleBrandSlug\". "
                        "register_module<Module, Info>() without a brand will not be available.")
    else()
        message(STATUS "${PLUGIN_NAME}: brand slug is '${PLUGIN_BRAND_SLUG}'")
        if (NOT PLUGIN_BRAND_SLUG STREQUAL PLUGIN_NAME)
            message(STATUS "${PLUGIN_NAME}: note: the plugin file name (${PLUGIN_NAME}.mmplugin) differs from the brand slug")
        endif()

        # Plugin code may be compiled in libraries linked into the plugin, too
        set(PLUGIN_SOURCE_TARGETS ${LIB_NAME})
        get_target_property(PLUGIN_LINKED_LIBS ${LIB_NAME} LINK_LIBRARIES)
        foreach(LIB IN LISTS PLUGIN_LINKED_LIBS)
            if (TARGET ${LIB} AND NOT LIB STREQUAL "arch_mp15x_a7" AND NOT LIB STREQUAL "metamodule-sdk")
                get_target_property(LIB_TYPE ${LIB} TYPE)
                get_target_property(LIB_IMPORTED ${LIB} IMPORTED)
                if (NOT LIB_IMPORTED AND (LIB_TYPE STREQUAL "STATIC_LIBRARY" OR LIB_TYPE STREQUAL "OBJECT_LIBRARY"))
                    list(APPEND PLUGIN_SOURCE_TARGETS ${LIB})
                endif()
            endif()
        endforeach()
        foreach(TGT IN LISTS PLUGIN_SOURCE_TARGETS)
            target_compile_definitions(${TGT} PRIVATE "METAMODULE_PLUGIN_BRAND_SLUG=\"${PLUGIN_BRAND_SLUG}\"")
        endforeach()
    endif()

    ###############


    target_link_libraries(${LIB_NAME} PRIVATE metamodule-sdk)

    # TODO: configure version file with SDK_MAJOR_VERSION etc
    set(VERSION_FILE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/version.cc)
    target_sources(${LIB_NAME} PRIVATE ${VERSION_FILE})

    # Route the plugin's exception unwinder to the host's unified exidx
    # lookup, so exceptions can unwind across the plugin/host boundary
    target_sources(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/unwind_exidx.c)

    target_sources(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/memalign_r.c)

    target_compile_definitions(${LIB_NAME} PRIVATE METAMODULE)
    target_compile_options(${LIB_NAME} PRIVATE 
        -fvisibility=hidden
        $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>
    )

    set(LFLAGS
        -shared
        -Wl,-Map,plugin.map,--cref
        -Wl,--gc-sections
        -Wl,--require-defined=init
        # Bind references to symbols defined in the plugin to the plugin's own
        # definition at link time. This matches how the MetaModule loader
        # resolves symbols anyway, and it lets place-relative references
        # (e.g. R_ARM_TARGET2/REL32 typeinfo refs in .ARM.extab) resolve
        # statically instead of becoming dynamic relocations the loader
        # does not support.
        -Wl,-Bsymbolic
        # Do not export symbols that come from static archives (plugin-libc,
        # libgcc): exported symbols are gc-sections roots, so without this the
        # .so keeps unreferenced library code alive (e.g. the transactional
        # memory clones in cow-stdexcept.cc, whose weak undefined _ITM_*
        # references the loader would report as missing symbols). The plugin's
        # own objects are linked directly (not via an archive), so init() and
        # sdk_version() remain exported.
        -Wl,--exclude-libs,ALL
        -nostartfiles 
        -nostdlib
        ${ARCH_MP15x_A7_FLAGS}
    )

    # libc/libm/libstdc++/libsupc++/unwinder, compiled -fPIC: the prebuilt
    # archive shipped with the SDK that matches the toolchain's gcc version.
    set(LIBC_ARCHIVE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plugin-libc/lib/libmetamodule-plugin-libc-gcc${METAMODULE_LIBC_GCC_MAJOR}.a)
    if (NOT EXISTS ${LIBC_ARCHIVE})
        message(FATAL_ERROR "Prebuilt plugin-libc archive not found: ${LIBC_ARCHIVE}\n"
            " Regenerate it with scripts/build-plugin-libc-autotools.sh")
    endif()
    set(LIBC_DEPENDS ${LIBC_ARCHIVE})

    # Get objects of linked libraries, except those we know about
    get_target_property(DEP_LIBS ${LIB_NAME} LINK_LIBRARIES)
    list(REMOVE_ITEM DEP_LIBS "arch_mp15x_a7")
    list(REMOVE_ITEM DEP_LIBS "metamodule-sdk")
    foreach(LIB IN LISTS DEP_LIBS)
        list(APPEND TARGET_LINK_LIB_OBJS $<TARGET_OBJECTS:${LIB}>)
    endforeach()

    # Link objects into a shared library (CMake won't do it for us)
    add_custom_command(
        OUTPUT ${PLUGIN_FILE_FULL}
        DEPENDS ${LIB_NAME} ${LIBC_DEPENDS}
        COMMAND ${CMAKE_CXX_COMPILER} ${LFLAGS} -o ${PLUGIN_FILE_FULL}
                $<TARGET_OBJECTS:${LIB_NAME}>
                ${TARGET_LINK_LIB_OBJS}
                ${LIBC_ARCHIVE}
                -lgcc
        COMMAND_EXPAND_LISTS
        VERBATIM USES_TERMINAL
    )

    # Strip symbols to create a smaller plugin file
    add_custom_command(
        OUTPUT ${PLUGIN_FILE_TMP}
        DEPENDS ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_STRIP} -g -v -o ${PLUGIN_FILE_TMP} ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_SIZE_UTIL} ${PLUGIN_FILE_TMP}
        VERBATIM USES_TERMINAL
    )
    add_custom_target(plugin ALL DEPENDS ${PLUGIN_FILE_TMP})

    # A plugin-mm.json with a syntax error fails validation, so don't package the plugin
    add_dependencies(plugin VALIDATE_PLUGIN_MM_JSON)

    # Verify symbols will be resolved
    set(FIRMWARE_SYMTAB_PATH ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/api-symbols.txt)
    add_custom_command(
        TARGET plugin
        POST_BUILD
        COMMAND scripts/check_syms.py 
            --plugin ${PLUGIN_FILE_TMP}
            --api ${FIRMWARE_SYMTAB_PATH}
            # -v
        WORKING_DIRECTORY ${CMAKE_CURRENT_FUNCTION_LIST_DIR}
        VERBATIM USES_TERMINAL
    )

    add_custom_command(
        TARGET plugin
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Creating plugin at ${PLUGIN_DEST_FILE}"
        # If any step below fails, don't leave the previous build's .mmplugin behind
        COMMAND ${CMAKE_COMMAND} -E rm -f ${PLUGIN_DEST_FILE}
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${PLUGIN_DEST_TMP_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_DEST_TMP_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_FILE_TMP} ${PLUGIN_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy ${PLUGIN_JSON_SOURCE} ${PLUGIN_JSON_DEST}
        # Rewrites any element-group members written as enumerator names into typed
        # indices, using the plugin's DWARF info. Everything else is copied as-is.
        COMMAND ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/resolve_element_groups.py
            --in ${PLUGIN_MM_JSON_SOURCE}
            --out ${PLUGIN_MM_JSON_DEST}
            --elf ${PLUGIN_FILE_FULL}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PLUGIN_OPTIONS_SOURCE_ASSETS} ${PLUGIN_DEST_TMP_DIR}
        COMMAND ${CMAKE_COMMAND} -E touch ${PLUGIN_DEST_TMP_DIR}/SDK-${SDK_MAJOR_VERSION}.${SDK_MINOR_VERSION}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_OPTIONS_PRESET_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PLUGIN_OPTIONS_PRESET_DIR} ${PLUGIN_DEST_TMP_DIR}/presets
        COMMAND ${CMAKE_COMMAND} -E rm -rf ${PLUGIN_DEST_TMP_DIR}/.DS_Store
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_DEST_DIR}
        COMMAND ${CMAKE_COMMAND} -E tar cf ${PLUGIN_DEST_FILE} ${PLUGIN_DEST_TMP_DIR}
        VERBATIM
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

