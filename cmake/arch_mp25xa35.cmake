# MetaModule MP2 (STM32MP2): Cortex-A35, AArch64. Same CPU flags as the MP2 firmware
# (firmware/cmake/arch_mp25xa35.cmake), so plugins and firmware agree on the ABI.
add_library(arch_mp25x_a35 INTERFACE)

set(ARCH_MP25x_A35_FLAGS
	-fno-math-errno
	-mcpu=cortex-a35
	-march=armv8-a+fp+simd
	-mtune=cortex-a35
	-mlittle-endian
)

target_link_options(arch_mp25x_a35 INTERFACE
	${ARCH_MP25x_A35_FLAGS}
)

target_compile_options(arch_mp25x_a35 INTERFACE
	${ARCH_MP25x_A35_FLAGS}
	"$<$<CONFIG:Debug>:-O0>"
	"$<$<CONFIG:Debug>:-g3>"
	"$<$<CONFIG:Release>:-O3>"
	"$<$<CONFIG:RelWithDebInfo>:-O3>"
	"$<$<CONFIG:RelWithDebInfo>:-g3>"
	-fno-common
	-fdata-sections
	-ffunction-sections
	-nostdlib
	-nostartfiles
	-Wall
	-Werror=return-type
	-Wno-psabi
	-Wno-double-promotion
	-Wno-attributes
	-shared
	-fPIC
	"$<$<COMPILE_LANGUAGE:CXX>:-Wno-register>"
	"$<$<COMPILE_LANGUAGE:CXX>:-Wno-volatile>"
	"$<$<COMPILE_LANGUAGE:CXX>:-ffold-simple-inlines>"
	"$<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>"
)
