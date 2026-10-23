cmake_minimum_required(VERSION 3.19)

# Which MetaModule the plugin is built for:
#  - mp1: STM32MP1 (Cortex-A7, 32-bit ARM, arm-none-eabi toolchain): MyPlugin.mmplugin
#  - mp2: STM32MP2 (Cortex-A35, AArch64, aarch64-none-elf toolchain): MyPlugin.mm2plugin
# A plugin project is configured for mp1, and its build also builds the MP2 plugin, with a second
# build of the same project for mp2 (see METAMODULE_BUILD_MP2 and create_plugin()).
set(METAMODULE_PLATFORM "mp1" CACHE STRING "MetaModule platform to build for: mp1 or mp2")
set_property(CACHE METAMODULE_PLATFORM PROPERTY STRINGS mp1 mp2)

if(METAMODULE_PLATFORM STREQUAL "mp1")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/arm-none-eabi-gcc.cmake)
elseif(METAMODULE_PLATFORM STREQUAL "mp2")
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/cmake/aarch64-none-elf-gcc.cmake)
else()
    message(FATAL_ERROR "METAMODULE_PLATFORM must be mp1 or mp2, not '${METAMODULE_PLATFORM}'")
endif()

project(MetaModulePluginSDK LANGUAGES C CXX ASM)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/version.cmake)

# Check ARM GCC toolchain version compatibility. Each supported major version
# has a matching prebuilt plugin-libc archive in plugin-libc/lib/.
# (MP2's plugin libc is made from the aarch64 toolchain's own libraries, so any version works.)
if(METAMODULE_PLATFORM STREQUAL "mp1" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
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
if(METAMODULE_PLATFORM STREQUAL "mp2")
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/arch_mp25xa35.cmake)
    set(METAMODULE_ARCH_TARGET arch_mp25x_a35)
    set(METAMODULE_ARCH_FLAGS ${ARCH_MP25x_A35_FLAGS})
    set(METAMODULE_PLUGIN_EXTENSION mm2plugin)
else()
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/arch_mp15xa7.cmake)
    set(METAMODULE_ARCH_TARGET arch_mp15x_a7)
    set(METAMODULE_ARCH_FLAGS ${ARCH_MP15x_A7_FLAGS})
    set(METAMODULE_PLUGIN_EXTENSION mmplugin)
endif()
link_libraries(${METAMODULE_ARCH_TARGET})

# The MP1 build also builds the MP2 plugin:
#   AUTO: if the aarch64-none-elf toolchain is found (in AARCH64_TOOLCHAIN_BASE_DIR, or the PATH)
#   ON:   always (an error if the toolchain isn't found, e.g. for release builds)
#   OFF:  never
if(METAMODULE_PLATFORM STREQUAL "mp1")
    set(METAMODULE_BUILD_MP2 "AUTO" CACHE STRING "Also build the MP2 plugin (.mm2plugin): AUTO, ON, or OFF")
    set_property(CACHE METAMODULE_BUILD_MP2 PROPERTY STRINGS AUTO ON OFF)
    set(AARCH64_TOOLCHAIN_BASE_DIR "" CACHE FILEPATH "Location of the aarch64-none-elf toolchain 'bin' directory (MP2)")
endif()

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

    # Path to final plugin tar file. (The MP2 build puts it where the MP1 build was told to.)
    if (DEFINED METAMODULE_PLUGIN_DESTINATION)
        set(PLUGIN_DEST_DIR ${METAMODULE_PLUGIN_DESTINATION})
    else()
        cmake_path(ABSOLUTE_PATH PLUGIN_OPTIONS_DESTINATION BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE PLUGIN_DEST_DIR)
    endif()
    cmake_path(APPEND PLUGIN_DEST_FILE ${PLUGIN_DEST_DIR} ${PLUGIN_NAME}.${METAMODULE_PLUGIN_EXTENSION})

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

    ###############


    target_link_libraries(${LIB_NAME} PRIVATE metamodule-sdk)

    # TODO: configure version file with SDK_MAJOR_VERSION etc
    set(VERSION_FILE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/version.cc)
    target_sources(${LIB_NAME} PRIVATE ${VERSION_FILE})

    if(METAMODULE_PLATFORM STREQUAL "mp1")
        # Route the plugin's exception unwinder to the host's unified exidx
        # lookup, so exceptions can unwind across the plugin/host boundary
        target_sources(${LIB_NAME} PRIVATE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/unwind_exidx.c)
    else()
        # MP2: these are in MP1's prebuilt plugin-libc archive, but not in the MP2 one (see
        # cmake/make_plugin_libc_mp2.cmake). The unwinder is the firmware's.
        target_sources(${LIB_NAME} PRIVATE
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plugin-libc/dso_handle.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plugin-libc/glue/vterminate_lite.cc
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plugin-libc/glue/mp2_glue.c
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plugin-libc/glue/mp2_locks.c
        )
    endif()

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
        ${METAMODULE_ARCH_FLAGS}
    )

    if(METAMODULE_PLATFORM STREQUAL "mp1")
        # libc/libm/libstdc++/libsupc++/unwinder, compiled -fPIC: the prebuilt
        # archive shipped with the SDK that matches the toolchain's gcc version.
        set(LIBC_ARCHIVE ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plugin-libc/lib/libmetamodule-plugin-libc-gcc${METAMODULE_LIBC_GCC_MAJOR}.a)
        if (NOT EXISTS ${LIBC_ARCHIVE})
            message(FATAL_ERROR "Prebuilt plugin-libc archive not found: ${LIBC_ARCHIVE}\n"
                " Regenerate it with scripts/build-plugin-libc-autotools.sh")
        endif()
        set(LIBC_DEPENDS ${LIBC_ARCHIVE})
        set(LIBGCC -lgcc)
        set(LINK_LAST_OBJS)
    else()
        # libc/libm/libstdc++/libsupc++/libgcc: made from the toolchain's own (see
        # cmake/make_plugin_libc_mp2.cmake). libgcc is in it, minus the unwinder (the firmware's).
        foreach(lib libc libm libstdc++ libgcc)
            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} ${METAMODULE_ARCH_FLAGS} -print-file-name=${lib}.a
                OUTPUT_VARIABLE path
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND_ERROR_IS_FATAL ANY)
            if (NOT IS_ABSOLUTE ${path})
                message(FATAL_ERROR "The aarch64 toolchain has no ${lib}.a")
            endif()
            set(TOOLCHAIN_${lib} ${path})
        endforeach()
        set(LIBC_ARCHIVE ${CMAKE_BINARY_DIR}/plugin-libc-mp2/libmetamodule-plugin-libc.a)
        set(LIBC_SCRIPT ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/cmake/make_plugin_libc_mp2.cmake)
        add_custom_command(
            OUTPUT ${LIBC_ARCHIVE}
            DEPENDS ${TOOLCHAIN_libc} ${TOOLCHAIN_libm} ${TOOLCHAIN_libstdc++} ${TOOLCHAIN_libgcc} ${LIBC_SCRIPT}
            COMMAND ${CMAKE_COMMAND}
                -DAR=${CMAKE_AR_PLAIN}
                -DLIBC=${TOOLCHAIN_libc}
                -DLIBM=${TOOLCHAIN_libm}
                -DLIBSTDCXX=${TOOLCHAIN_libstdc++}
                -DLIBGCC=${TOOLCHAIN_libgcc}
                -DOUT=${LIBC_ARCHIVE}
                -DWORK_DIR=${CMAKE_BINARY_DIR}/plugin-libc-mp2/work
                -P ${LIBC_SCRIPT}
            COMMENT "Making the MP2 plugin libc from the toolchain's libraries"
            VERBATIM
        )
        set(LIBC_DEPENDS ${LIBC_ARCHIVE})
        set(LIBGCC)
        # The AArch64 linker aligns segments to 64kB pages by default: the loader's code buffer is
        # aligned to 4kB, and 4kB pages also keep the plugin's image smaller
        list(APPEND LFLAGS -Wl,-z,max-page-size=0x1000 -Wl,-z,common-page-size=0x1000)
        # Ends .eh_frame: must be the last object linked
        set(LINK_LAST_OBJS ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/eh_frame_end.S)
    endif()

    # Get objects of linked libraries, except those we know about
    get_target_property(DEP_LIBS ${LIB_NAME} LINK_LIBRARIES)
    list(REMOVE_ITEM DEP_LIBS ${METAMODULE_ARCH_TARGET})
    list(REMOVE_ITEM DEP_LIBS "metamodule-sdk")
    foreach(LIB IN LISTS DEP_LIBS)
        list(APPEND TARGET_LINK_LIB_OBJS $<TARGET_OBJECTS:${LIB}>)
    endforeach()

    # Link objects into a shared library (CMake won't do it for us)
    add_custom_command(
        OUTPUT ${PLUGIN_FILE_FULL}
        DEPENDS ${LIB_NAME} ${LIBC_DEPENDS} ${LINK_LAST_OBJS}
        COMMAND ${CMAKE_CXX_COMPILER} ${LFLAGS} -o ${PLUGIN_FILE_FULL}
                $<TARGET_OBJECTS:${LIB_NAME}>
                ${TARGET_LINK_LIB_OBJS}
                ${LIBC_ARCHIVE}
                ${LIBGCC}
                ${LINK_LAST_OBJS}
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
    if(METAMODULE_PLATFORM STREQUAL "mp2")
        set(FIRMWARE_SYMTAB_PATH ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/api-symbols-mp2.txt)
    else()
        set(FIRMWARE_SYMTAB_PATH ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/api-symbols.txt)
    endif()
    if(EXISTS ${FIRMWARE_SYMTAB_PATH})
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
    else()
        message(WARNING "${FIRMWARE_SYMTAB_PATH} not found: can't check that the firmware provides the symbols the plugin uses")
    endif()

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

    if(METAMODULE_PLATFORM STREQUAL "mp1")
        build_mp2_plugin(${PLUGIN_NAME} ${PLUGIN_DEST_DIR})
    endif()

endfunction()

# The MP2 plugin: a second build of this project (the same CMakeLists.txt) for mp2, which puts the
# .mm2plugin next to the .mmplugin. It's built along with the MP1 plugin.
function(build_mp2_plugin PLUGIN_NAME PLUGIN_DEST_DIR)
    if(METAMODULE_BUILD_MP2 STREQUAL "OFF")
        return()
    endif()

    if(AARCH64_TOOLCHAIN_BASE_DIR)
        set(AARCH64_GXX ${AARCH64_TOOLCHAIN_BASE_DIR}/aarch64-none-elf-g++)
        if(NOT EXISTS ${AARCH64_GXX})
            unset(AARCH64_GXX)
        endif()
    else()
        find_program(AARCH64_GXX aarch64-none-elf-g++)
    endif()

    if(NOT AARCH64_GXX)
        set(msg "The aarch64-none-elf toolchain wasn't found, so the MP2 plugin (${PLUGIN_NAME}.mm2plugin) "
            "won't be built. Install the AArch64 bare-metal (aarch64-none-elf) toolchain from "
            "https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads and put it in your PATH, "
            "or set AARCH64_TOOLCHAIN_BASE_DIR to its bin directory. (METAMODULE_BUILD_MP2=OFF silences this.)")
        if(METAMODULE_BUILD_MP2 STREQUAL "ON")
            message(FATAL_ERROR ${msg})
        else()
            message(WARNING ${msg})
            return()
        endif()
    endif()

    # Pass on the cache variables the plugin's project may use (e.g. its own options, set with -D)
    get_cmake_property(cache_vars CACHE_VARIABLES)
    set(cache_args)
    foreach(var ${cache_vars})
        get_property(type CACHE ${var} PROPERTY TYPE)
        if(type STREQUAL "INTERNAL" OR type STREQUAL "STATIC")
            continue()
        endif()
        if(var MATCHES "^CMAKE_" OR var MATCHES "^METAMODULE_PLATFORM$|^METAMODULE_BUILD_MP2$|^TOOLCHAIN_BASE_DIR$")
            continue()
        endif()
        if(type STREQUAL "UNINITIALIZED")
            set(type STRING)
        endif()
        list(APPEND cache_args "-D${var}:${type}=${${var}}")
    endforeach()

    include(ExternalProject)
    ExternalProject_Add(${PLUGIN_NAME}-mp2
        SOURCE_DIR ${CMAKE_SOURCE_DIR}
        BINARY_DIR ${CMAKE_BINARY_DIR}/mp2
        CMAKE_CACHE_ARGS
            ${cache_args}
            -DMETAMODULE_PLATFORM:STRING=mp2
            -DMETAMODULE_PLUGIN_DESTINATION:PATH=${PLUGIN_DEST_DIR}
        INSTALL_COMMAND ""
        BUILD_ALWAYS ON
        USES_TERMINAL_CONFIGURE ON
        USES_TERMINAL_BUILD ON
    )
endfunction()

