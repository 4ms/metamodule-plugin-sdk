cmake_minimum_required(VERSION 3.19)

# Makes the MP2 plugin libc archive from the aarch64-none-elf toolchain's own libc, libm, libstdc++
# and libgcc (run by create_plugin() with cmake -P).
#
# Unlike MP1 (plugin-libc/), these needn't be rebuilt with -fPIC: AArch64 code addresses data
# PC-relative, so the toolchain's archives link into a plugin as they are, as long as the data they
# refer to is in the plugin too (non-PIC code can't refer to data in the firmware: so, unlike MP1,
# each plugin has its own _impure_ptr, i.e. its own stdio state and errno). Functions can be
# imported from the firmware. What has to change is which functions the plugin imports:
#  - malloc family, abort: the firmware's heap and abort
#  - _*_r reentrant syscalls: the firmware's
#    (MP1 also imports init/fini/__call_atexit, but they refer to data (__atexit,
#    __libc_fini_array), so they stay in the plugin: __cxa_atexit registrations go to the plugin's
#    own list, and nothing calls them, as on MP1. The loader runs .init_array itself.)
#  - stat: the firmware exports stat() but not _stat/_stat_r (see MP1's
#    scripts/build-plugin-libc-autotools.sh)
#  - the DWARF unwinder (libgcc's unwind-dw2*): the firmware's, which the loader registers each
#    plugin's .eh_frame with, so an exception can unwind through plugin and firmware frames
#  - vterminate/cp-demangle: the plugin gets the SDK's lightweight terminate handler instead
#    (plugin-libc/glue/vterminate_lite.cc), not the 100kB demangler
#
# Inputs: AR, LIBC, LIBM, LIBSTDCXX, LIBGCC, OUT, WORK_DIR

foreach(var AR LIBC LIBM LIBSTDCXX LIBGCC OUT WORK_DIR)
	if(NOT DEFINED ${var})
		message(FATAL_ERROR "make_plugin_libc_mp2.cmake: ${var} is not set")
	endif()
endforeach()

file(REMOVE_RECURSE ${WORK_DIR})
file(MAKE_DIRECTORY ${WORK_DIR})

file(COPY_FILE ${LIBC} ${WORK_DIR}/libc.a)
file(COPY_FILE ${LIBM} ${WORK_DIR}/libm.a)
file(COPY_FILE ${LIBSTDCXX} ${WORK_DIR}/libstdc++.a)
file(COPY_FILE ${LIBGCC} ${WORK_DIR}/libgcc.a)

# Deletes archive members by name. A member that isn't there is an error: a rename in a new
# toolchain must be noticed, not skipped.
function(delete_members archive)
	execute_process(
		COMMAND ${AR} t ${archive}
		WORKING_DIRECTORY ${WORK_DIR}
		OUTPUT_VARIABLE members
		COMMAND_ERROR_IS_FATAL ANY)
	string(REPLACE "\n" ";" members "${members}")
	foreach(member ${ARGN})
		if(NOT member IN_LIST members)
			message(FATAL_ERROR "Expected member ${member} not found in the toolchain's ${archive}: "
				"the member list in make_plugin_libc_mp2.cmake needs updating for this toolchain")
		endif()
	endforeach()
	execute_process(
		COMMAND ${AR} d ${archive} ${ARGN}
		WORKING_DIRECTORY ${WORK_DIR}
		COMMAND_ERROR_IS_FATAL ANY)
endfunction()

delete_members(libc.a
	libc_a-sysstat.o libc_a-statr.o
	libc_a-malloc.o libc_a-mallocr.o libc_a-calloc.o libc_a-callocr.o
	libc_a-realloc.o libc_a-reallocr.o libc_a-freer.o
	libc_a-malign.o libc_a-malignr.o libc_a-msize.o
	libc_a-abort.o
	libc_a-closer.o libc_a-fstatr.o libc_a-gettimeofdayr.o
	libc_a-isattyr.o libc_a-lseekr.o libc_a-openr.o libc_a-readr.o
	libc_a-sbrkr.o libc_a-signalr.o libc_a-timesr.o libc_a-writer.o)

delete_members(libgcc.a unwind-dw2.o unwind-dw2-fde.o unwind-sjlj.o)

delete_members(libstdc++.a vterminate.o cp-demangle.o)

# Merge them into one archive
file(WRITE ${WORK_DIR}/merge.mri
	"CREATE ${OUT}\n"
	"ADDLIB libstdc++.a\n"
	"ADDLIB libc.a\n"
	"ADDLIB libm.a\n"
	"ADDLIB libgcc.a\n"
	"SAVE\n"
	"END\n")
file(REMOVE ${OUT})
execute_process(
	COMMAND ${AR} -M
	INPUT_FILE ${WORK_DIR}/merge.mri
	WORKING_DIRECTORY ${WORK_DIR}
	COMMAND_ERROR_IS_FATAL ANY)
