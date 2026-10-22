# Toolchain for MetaModule MP2 (STM32MP2, Cortex-A35, AArch64) plugins
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(AARCH64_TOOLCHAIN_BASE_DIR "" CACHE FILEPATH "Location of the aarch64-none-elf toolchain 'bin' directory (MP2)")

if (AARCH64_TOOLCHAIN_BASE_DIR)
  set(TOOLCHAIN_PREFIX "${AARCH64_TOOLCHAIN_BASE_DIR}/aarch64-none-elf-")
else()
  set(TOOLCHAIN_PREFIX "aarch64-none-elf-")
endif()

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_STRIP        ${TOOLCHAIN_PREFIX}strip   CACHE INTERNAL "strip tool")
set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump CACHE INTERNAL "objdump tool")
set(CMAKE_READELF      ${TOOLCHAIN_PREFIX}readelf CACHE INTERNAL "readelf tool")
set(CMAKE_NM           ${TOOLCHAIN_PREFIX}nm      CACHE INTERNAL "nm tool")
set(CMAKE_SIZE_UTIL    ${TOOLCHAIN_PREFIX}size    CACHE INTERNAL "size tool")
set(CMAKE_RANLIB       ${TOOLCHAIN_PREFIX}ranlib  CACHE INTERNAL "ranlib tool")
set(CMAKE_AR_PLAIN     ${TOOLCHAIN_PREFIX}ar      CACHE INTERNAL "ar tool (not the gcc-ar wrapper)")
find_program(CMAKE_GDB ${TOOLCHAIN_PREFIX}gdb gdb CACHE INTERNAL "gdb client")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Handle static libraries through compiler (for LTO data)
SET(CMAKE_AR  ${TOOLCHAIN_PREFIX}gcc-ar)

set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE
"-Wl,--whole-archive" "<LINK_ITEM>" "-Wl,--no-whole-archive"
)
set(CMAKE_LINK_LIBRARY_USING_WHOLE_ARCHIVE_SUPPORTED True)

set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> -xassembler-with-cpp -o <OBJECT> -c <SOURCE> ")
