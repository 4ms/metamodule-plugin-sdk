#!/usr/bin/env bash
#
# Build plugin-libc from the real newlib and libstdc++ autotools build
# systems, matching the newlib/libstdc++ versions shipped in a supported ARM
# GNU toolchain release (12.2/12.3, 13.2/13.3, 14.2/14.3, or 15.2/15.3).
#
# Produces plugin-libc/autotools-build/gcc<N>/libmetamodule-plugin-libc-gcc<N>.a
# -- a drop-in replacement for the archive in plugin-libc/lib/ (it is NOT
# installed there automatically; test on hardware first, then copy manually).
#
# What this does:
#  1. Downloads the newlib source tarball matching the toolchain's newlib
#     version, and the gcc source tarball matching the toolchain's gcc (for
#     libstdc++-v3 and the libgcc unwinder sources)
#  2. Builds newlib with CFLAGS_FOR_TARGET += -fPIC and configure flags that
#     reproduce the installed toolchain's configuration exactly -- the feature
#     options are read back out of its newlib.h, since distro-packaged
#     toolchains configure newlib differently than the ARM GNU releases do
#     (verified by diffing the generated newlib.h against the installed copy)
#  3. Builds libstdc++-v3 standalone (--host=arm-none-eabi) with -fPIC and
#     --with-pic, using the installed cross compiler. Configure link-tests
#     are impossible for bare metal (GCC_NO_EXECUTABLES), so the answers are
#     seeded to match the toolchain's installed c++config.h
#  4. Compiles the ARM EABI unwinder (from the gcc tarball's libgcc/) + glue
#  5. Removes archive members whose symbols must bind to the firmware at
#     load time (malloc family, abort, the reentrant _*_r syscalls, init/fini
#     hooks)
#  6. Replaces libstdc++'s __verbose_terminate_handler (which drags in the
#     ~100kB demangler) with glue/vterminate_lite.cc
#  7. Merges everything into one libmetamodule-plugin-libc-gcc<N>.a
#
# Usage:
#   scripts/build-plugin-libc-autotools.sh [12|13|14|15] [/path/to/arm-gnu-toolchain/bin]
#
# Arguments may be given in either order. With a bare version number, the
# arm-none-eabi-gcc found on PATH must be that version. With a toolchain
# path, the version is detected from it. With no arguments, the PATH
# toolchain is used and its version detected.
#
set -euo pipefail

SDK_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BASE="$SDK_DIR/plugin-libc/autotools-build"

##############################################################################
# Argument parsing: a supported gcc major version and/or a toolchain bin dir
REQUESTED_VER=""
TOOLCHAIN_BASE_DIR="${TOOLCHAIN_BASE_DIR:-}"
for arg in "$@"; do
	case "$arg" in
		12|13|14|15) REQUESTED_VER="$arg" ;;
		*)
			if [ -d "$arg" ]; then
				TOOLCHAIN_BASE_DIR="$arg"
			else
				echo "ERROR: argument '$arg' is neither a supported gcc version (12/13/14/15) nor a directory" >&2
				exit 1
			fi
			;;
	esac
done

if [ -z "$TOOLCHAIN_BASE_DIR" ]; then
	TOOLCHAIN_BASE_DIR="$(dirname "$(command -v arm-none-eabi-gcc)")"
fi
TC="$TOOLCHAIN_BASE_DIR"

# The whole build must use the matching toolchain. A mismatched
# arm-none-eabi-gcc can miscompile or ICE on libsupc++ (seen with 14.2:
# "internal compiler error: in gimple_build_eh_must_not_throw").
export PATH="$TC:$PATH"
GCCVER=$("$TC/arm-none-eabi-gcc" -dumpversion)
case "$GCCVER" in
	12.2*|12.3*) GCC_MAJOR=12; SRC_VARIANT=12.3 ;;
	13.2*)       GCC_MAJOR=13; SRC_VARIANT=13.2 ;;
	13.3*)       GCC_MAJOR=13; SRC_VARIANT=13.3 ;;
	14.2*)       GCC_MAJOR=14; SRC_VARIANT=14.2 ;;
	14.3*)       GCC_MAJOR=14; SRC_VARIANT=14.3 ;;
	15.2*)       GCC_MAJOR=15; SRC_VARIANT=15.2 ;;
	15.3*)       GCC_MAJOR=15; SRC_VARIANT=15.3 ;;
	*) echo "ERROR: arm-none-eabi-gcc $GCCVER at $TC; need 12.2/12.3, 13.2/13.3, 14.2/14.3, or 15.2/15.3" >&2; exit 1 ;;
esac
if [ -n "$REQUESTED_VER" ] && [ "$REQUESTED_VER" != "$GCC_MAJOR" ]; then
	echo "ERROR: gcc $REQUESTED_VER requested, but arm-none-eabi-gcc at $TC is $GCCVER." >&2
	echo "       Pass the path to a gcc $REQUESTED_VER toolchain's bin/ directory." >&2
	exit 1
fi

##############################################################################
# Where the toolchain keeps its target headers -- i.e. what the rest of this
# script means by "$sysroot/include".
#
# ARM's official toolchains answer -print-sysroot with <toolchain>/arm-none-eabi
# and put the headers in $sysroot/include. Distro packages (Debian/Ubuntu
# gcc-arm-none-eabi, and the arm-none-eabi-gcc in most other distros) are
# configured without a sysroot: -print-sysroot prints an empty string and the
# same headers live in /usr/lib/arm-none-eabi/include. So ask the preprocessor
# where it actually looks rather than assuming either layout.
find_tc_include() {
	local sysroot dir
	sysroot="$("$TC/arm-none-eabi-gcc" -print-sysroot)"
	if [ -n "$sysroot" ] && [ -f "$sysroot/include/newlib.h" ]; then
		echo "$sysroot/include"
		return 0
	fi
	# The entries of the '#include <...>' search list are the output lines
	# indented by one space. gcc's own include/ and include-fixed/ come
	# first but never hold newlib.h, so testing for it picks the target
	# header dir.
	while read -r dir; do
		[ -f "$dir/newlib.h" ] || continue
		(cd "$dir" && pwd)
		return 0
	done < <("$TC/arm-none-eabi-gcc" -E -Wp,-v -xc /dev/null -o /dev/null 2>&1 |
	         sed -n 's/^ \([^ ].*\)$/\1/p')
	return 1
}
if ! TC_INCLUDE="$(find_tc_include)"; then
	echo "ERROR: could not find newlib.h in the include search path of" >&2
	echo "       $TC/arm-none-eabi-gcc." >&2
	echo "       The toolchain's newlib headers are missing -- on Debian/Ubuntu" >&2
	echo "       install the libnewlib-arm-none-eabi package." >&2
	exit 1
fi

##############################################################################
# Source versions: newlib matching what the toolchain ships (its
# $TC_INCLUDE/_newlib_version.h), gcc matching its gcc.
case "$SRC_VARIANT" in
	12.3) NEWLIB_RELEASE=4.3.0; GCC_VER=gcc-12.3.0 ;;
	13.2) NEWLIB_RELEASE=4.3.0; GCC_VER=gcc-13.2.0 ;;
	13.3) NEWLIB_RELEASE=4.4.0; GCC_VER=gcc-13.3.0 ;;
	14.2) NEWLIB_RELEASE=4.4.0; GCC_VER=gcc-14.2.0 ;;
	14.3) NEWLIB_RELEASE=4.5.0; GCC_VER=gcc-14.3.0 ;;
	15.2) NEWLIB_RELEASE=4.5.0; GCC_VER=gcc-15.2.0 ;;
	15.3) NEWLIB_RELEASE=4.6.0; GCC_VER=gcc-15.3.0 ;;
esac

# Those are the pairings of the ARM GNU toolchain releases. Distro packages
# pair the same gcc with a different newlib (Debian's 13.2.1 ships newlib
# 4.4.0, not the 4.3.0 of ARM's 13.2.rel1), so believe the toolchain over the
# table -- building against the wrong newlib sources would fail the newlib.h
# check below at best, and silently produce a mismatched libc at worst.
TC_NEWLIB="$(sed -n 's/^#define _NEWLIB_VERSION "\(.*\)"/\1/p' \
	"$TC_INCLUDE/_newlib_version.h" 2>/dev/null || true)"
if [ -n "$TC_NEWLIB" ] && [ "$TC_NEWLIB" != "$NEWLIB_RELEASE" ]; then
	echo "==== note: this toolchain ships newlib $TC_NEWLIB (ARM's gcc $SRC_VARIANT"
	echo "     release ships $NEWLIB_RELEASE); building newlib $TC_NEWLIB to match."
	NEWLIB_RELEASE="$TC_NEWLIB"
fi

case "$NEWLIB_RELEASE" in
	4.3.0) NEWLIB_VER=newlib-4.3.0.20230120 ;;
	4.4.0) NEWLIB_VER=newlib-4.4.0.20231231 ;;
	4.5.0) NEWLIB_VER=newlib-4.5.0.20241231 ;;
	4.6.0) NEWLIB_VER=newlib-4.6.0.20260123 ;;
	*) echo "ERROR: no known source tarball for newlib $NEWLIB_RELEASE (shipped by" >&2
	   echo "       $TC/arm-none-eabi-gcc) -- add it to the table in $0" >&2
	   exit 1 ;;
esac
NEWLIB_URL="https://sourceware.org/pub/newlib/${NEWLIB_VER}.tar.gz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/${GCC_VER}/${GCC_VER}.tar.xz"

##############################################################################
# Newlib's feature options, read back from the toolchain's own newlib.h.
#
# These are not the same across toolchains: ARM's releases enable multibyte
# support, C99 IO formats and retargetable locking; Debian's
# gcc-arm-none-eabi enables none of the three. Every one of them leaves its
# fingerprint in newlib.h, so take the toolchain's answer rather than
# hardcoding one vendor's choices -- the archive has to agree with the
# headers plugins are compiled against. Step 2a below diffs the whole file,
# so an option not listed here is still caught rather than silently wrong.
newlib_opt() { # <macro in newlib.h> <configure option>
	if grep -q "^#define $1\b" "$TC_INCLUDE/newlib.h"; then
		echo "--enable-$2"
	else
		echo "--disable-$2"
	fi
}
NEWLIB_OPTS=(
	"$(newlib_opt _WANT_IO_LONG_LONG    newlib-io-long-long)"
	"$(newlib_opt _WANT_IO_C99_FORMATS  newlib-io-c99-formats)"
	"$(newlib_opt _MB_CAPABLE           newlib-mb)"
	"$(newlib_opt _REENT_CHECK_VERIFY   newlib-reent-check-verify)"
	"$(newlib_opt _WANT_REGISTER_FINI   newlib-register-fini)"
	"$(newlib_opt _RETARGETABLE_LOCKING newlib-retargetable-locking)"
)

# Tarballs and extracted source trees are shared (version-named) in $BASE;
# everything built lives in a per-gcc-version work dir so archives for
# several versions can be (re)built side by side. The archive is named by
# major version only: plugins must be built with the same toolchain the
# archive was generated with, and plugin.cmake selects by major version.
WORK="$BASE/gcc$SRC_VARIANT"
OUT_NAME="libmetamodule-plugin-libc-gcc$GCC_MAJOR.a"
# The newlib version is not implied by the gcc version (distro toolchains
# pair them differently), so the newlib build dir is named after its sources:
# switching toolchains must not silently reuse a tree configured from another
# newlib release.
NEWLIB_BUILD="build-$NEWLIB_VER"

ARCH_FLAGS="-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -mthumb-interwork -mno-unaligned-access -mtune=cortex-a7"
PIC_FLAGS="-fPIC -ffunction-sections -fdata-sections -O2 -g"

NPROC="$(sysctl -n hw.ncpu 2>/dev/null || nproc)"
# Recursive `make -j` on the newlib / libstdc++ autotools trees occasionally
# dies on a transient parallel-build race -- more likely on many-core
# machines, which is why this can fail on one mac and succeed on another. The
# build is fully resumable, so retry a couple of times (each retry just
# continues where the last left off) before treating it as a real failure.
run_make() {
	local tries=0
	until make -j"$NPROC"; do
		tries=$((tries + 1))
		if [ "$tries" -ge 3 ]; then
			echo "ERROR: 'make' failed $tries times in $(pwd) -- this is a real" >&2
			echo "       build error, not a parallel race. Re-run with 'make' (no" >&2
			echo "       -j) here to see the failing command clearly." >&2
			return 1
		fi
		echo "==== 'make' failed (likely a -j$NPROC parallel race); retrying ($tries/2)..." >&2
	done
}

mkdir -p "$WORK"

# Mirror all stdout/stderr to a logfile. The recursive -j build produces a lot
# of interleaved output, so when something fails the error scrolls off the
# terminal -- grep this file (e.g. for 'Error ' or 'error:') to find it.
BUILD_LOG="$WORK/build.log"
exec > >(tee "$BUILD_LOG") 2>&1
echo "==== building with $("$TC/arm-none-eabi-gcc" --version | head -1)"
echo "==== full build output is being saved to $BUILD_LOG"

##############################################################################
echo "==== 1. Sources"
cd "$BASE"
[ -f ${NEWLIB_VER}.tar.gz ] || curl -L -o ${NEWLIB_VER}.tar.gz "$NEWLIB_URL"
[ -f ${GCC_VER}.tar.xz ]    || curl -L -o ${GCC_VER}.tar.xz "$GCC_URL"
[ -d ${NEWLIB_VER} ] || tar xzf ${NEWLIB_VER}.tar.gz
# libstdc++-v3 needs these pieces of the gcc tree (but never builds gcc itself):
[ -d ${GCC_VER} ] || tar xJf ${GCC_VER}.tar.xz \
	${GCC_VER}/libstdc++-v3 ${GCC_VER}/config ${GCC_VER}/libgcc \
	${GCC_VER}/libiberty ${GCC_VER}/include ${GCC_VER}/contrib \
	${GCC_VER}/libbacktrace \
	${GCC_VER}/gcc/BASE-VER ${GCC_VER}/gcc/DATESTAMP \
	${GCC_VER}/config.guess ${GCC_VER}/config.sub ${GCC_VER}/install-sh \
	${GCC_VER}/ltmain.sh ${GCC_VER}/libtool-ldflags ${GCC_VER}/missing \
	${GCC_VER}/config-ml.in \
	${GCC_VER}/depcomp ${GCC_VER}/mkinstalldirs

BUILD_TRIPLET=$(${GCC_VER}/config.guess)
cd "$WORK"

##############################################################################
echo "==== 2. newlib"
# Configure flags reproduce the installed toolchain's newlib configuration
# (validated below by diffing the generated newlib.h).
mkdir -p "$NEWLIB_BUILD" && cd "$NEWLIB_BUILD"
# $NEWLIB_OPTS depends on the toolchain, not just on the newlib version in the
# dir name, so a tree left over from a differently-configured toolchain must be
# reconfigured rather than reused.
if [ -f Makefile ] && [ "$(cat .newlib-opts 2>/dev/null || true)" != "${NEWLIB_OPTS[*]}" ]; then
	echo "==== newlib configure options changed since this tree was configured; rebuilding it"
	cd .. && rm -rf "$NEWLIB_BUILD" && mkdir -p "$NEWLIB_BUILD" && cd "$NEWLIB_BUILD"
fi
if [ ! -f Makefile ]; then
"$BASE"/${NEWLIB_VER}/configure \
	--build=$BUILD_TRIPLET --host=$BUILD_TRIPLET --target=arm-none-eabi \
	--prefix="$WORK/install" \
	--disable-multilib --disable-nls \
	--disable-newlib-supplied-syscalls \
	"${NEWLIB_OPTS[@]}" \
	CC_FOR_TARGET="$TC/arm-none-eabi-gcc" \
	GCC_FOR_TARGET="$TC/arm-none-eabi-gcc" \
	CXX_FOR_TARGET="$TC/arm-none-eabi-g++" \
	AR_FOR_TARGET="$TC/arm-none-eabi-gcc-ar" \
	RANLIB_FOR_TARGET="$TC/arm-none-eabi-gcc-ranlib" \
	CFLAGS_FOR_TARGET="$ARCH_FLAGS $PIC_FLAGS"
echo "${NEWLIB_OPTS[*]}" > .newlib-opts
fi
run_make
cd "$WORK"

echo "==== 2a. verify newlib.h matches the toolchain's"
GEN_NEWLIB_H=$(find "$NEWLIB_BUILD" -name newlib.h -path "*targ-include*" | head -1)
# _EXECL_USE_MALLOC and _HAVE_HW_MISALIGNED_ACCESS are ignored: ARM's 14.3
# toolchain ships a newlib snapshot slightly newer than the 4.5.0 release
# tarball, whose newlib.h template mentions these two (disabled) options that
# the release's template does not. Both configurations leave them undefined,
# so the resulting library is identical.
if ! diff <(grep -E "^#define|^/\* #undef" "$TC_INCLUDE/newlib.h" | grep -v "VERSION\|PATCHLEVEL\|_EXECL_USE_MALLOC\|_HAVE_HW_MISALIGNED_ACCESS" | sort) \
          <(grep -E "^#define|^/\* #undef" "$GEN_NEWLIB_H"              | grep -v "VERSION\|PATCHLEVEL\|_EXECL_USE_MALLOC\|_HAVE_HW_MISALIGNED_ACCESS" | sort); then
	echo "ERROR: generated newlib.h differs from the toolchain's (left column is" >&2
	echo "       $TC_INCLUDE/newlib.h, right is what this build produced)." >&2
	echo "       The feature options in \$NEWLIB_OPTS are taken from the toolchain" >&2
	echo "       automatically; a difference here is an option this script does not" >&2
	echo "       know about yet -- map the macro above to its --enable-newlib-* flag" >&2
	echo "       and add it to \$NEWLIB_OPTS." >&2
	exit 1
fi
echo "newlib.h: OK (identical configuration)"

##############################################################################
echo "==== 3. libstdc++"
# Link tests are not possible (bare metal, GCC_NO_EXECUTABLES), so probe
# answers that need them are seeded to match the toolchain's installed
# c++config.h (arm-none-eabi/include/c++/*/arm-none-eabi/*/bits/c++config.h).
# The result is verified against that file below.
#
# The arch flags and -fPIC are part of $CC/$CXX (not CFLAGS/CXXFLAGS):
# some configure probes replace CXXFLAGS entirely (the atomic-builtins asm
# probe sets CXXFLAGS='-O0 -S'), and without -mcpu the probe miscompiles
# for the default architecture and concludes there are no atomic builtins.
# This is also how the in-tree multilib build passes arch flags.
# Extra seeds for probes that only exist in newer libstdc++ versions:
#  - the chdir/chmod/getcwd/mkdir link-tests (gcc >= 13) fail on bare metal
#    (the ARM toolchain's c++config.h has them undefined)
#  - std::stacktrace support (in libstdc++exp.a, not merged into our archive)
#    is enabled in the ARM toolchains from 14.3 on, and must match in
#    c++config.h (13.3 ships with it disabled)
EXTRA_LIBSTDCXX_ARGS=""
if [ "$GCC_MAJOR" -ge 13 ]; then
	EXTRA_LIBSTDCXX_ARGS="glibcxx_cv_chdir=no glibcxx_cv_chmod=no
		glibcxx_cv_getcwd=no glibcxx_cv_mkdir=no"
fi
if [ "$GCC_MAJOR" -ge 14 ]; then
	EXTRA_LIBSTDCXX_ARGS="$EXTRA_LIBSTDCXX_ARGS --enable-libstdcxx-backtrace=yes"
fi
mkdir -p build-libstdcxx && cd build-libstdcxx
if [ ! -f Makefile ]; then
"$BASE"/${GCC_VER}/libstdc++-v3/configure \
	--host=arm-none-eabi --build=$BUILD_TRIPLET \
	--prefix="$WORK/install" \
	--disable-shared --disable-multilib --disable-nls \
	--with-newlib --disable-libstdcxx-pch \
	--with-pic \
	ac_cv_func_fcntl=yes ac_cv_func_getexecname=no ac_cv_func__wfopen=no \
	ac_cv_func_secure_getenv=no ac_cv_func_setenv=no ac_cv_func_quick_exit=no \
	ac_cv_func_at_quick_exit=no ac_cv_func_timespec_get=no ac_cv_func_sockatmark=no \
	ac_cv_func_uselocale=no ac_cv_func_sincos=no ac_cv_func__sincos=no \
	ac_cv_func_strtof=yes ac_cv_func_strtold=no \
	ac_cv_func___cxa_thread_atexit=no ac_cv_func___cxa_thread_atexit_impl=no \
	glibcxx_cv_c99_complex_cxx98=yes glibcxx_cv_c99_complex_cxx11=yes \
	glibcxx_cv_getentropy=no glibcxx_cv_arc4random=no \
	glibcxx_cv_openat=no glibcxx_cv_unlinkat=no \
	glibcxx_cv_readlink=no glibcxx_cv_symlink=no \
	glibcxx_cv_fchmod=no glibcxx_cv_fchmodat=no \
	$EXTRA_LIBSTDCXX_ARGS \
	CC="$TC/arm-none-eabi-gcc $ARCH_FLAGS -fPIC" \
	CXX="$TC/arm-none-eabi-g++ $ARCH_FLAGS -fPIC" \
	CFLAGS="-ffunction-sections -fdata-sections -O2 -g" \
	CXXFLAGS="-ffunction-sections -fdata-sections -O2 -g"

# The complex.h inverse-trig probe (gcc >= 14) is not a cache variable, so it
# cannot be seeded. It fails here only because the standalone probe resolves
# #include <complex.h> to the *installed* toolchain's C++ <complex.h> wrapper
# (which does not declare cacosf & co.), whereas the in-tree toolchain build
# resolved it to newlib's C header (which does; newlib's libm provides them).
# Fix up config.h to match the toolchain; the c++config.h verification below
# guards this.
if grep -q '/\* #undef _GLIBCXX_USE_C99_COMPLEX_ARC \*/' config.h; then
	sed -i.bak 's,/\* #undef _GLIBCXX_USE_C99_COMPLEX_ARC \*/,#define _GLIBCXX_USE_C99_COMPLEX_ARC 1,' config.h
fi
fi
run_make
cd "$WORK"

echo "==== 3a. verify c++config.h matches the toolchain's"
# Everything except the __GLIBCXX__ datestamp (the .0 release vs the
# toolchain's .1 branch snapshot) must be identical, otherwise the
# archive's internals disagree with the headers plugins compile against.
# (This catches, e.g., std::random_device referencing getentropy that the
# firmware does not provide.)
#
# _GLIBCXX_HAVE_O_NONBLOCK is ignored: ARM's 14.2 toolchain snapshot
# backported that configure probe, which the gcc-14.2.0 release tarball
# lacks. The macro is never referenced anywhere in libstdc++ (headers or
# src), so the difference is cosmetic.
MULTIDIR=$("$TC/arm-none-eabi-gcc" $ARCH_FLAGS -print-multi-directory)
TC_CXXCONF="$TC_INCLUDE/c++/$("$TC/arm-none-eabi-gcc" -dumpversion)/arm-none-eabi/$MULTIDIR/bits/c++config.h"
if [ ! -f "$TC_CXXCONF" ]; then
	echo "ERROR: the toolchain's c++config.h is not at $TC_CXXCONF" >&2
	echo "       (on Debian/Ubuntu it comes from libstdc++-arm-none-eabi-dev)" >&2
	exit 1
fi
GEN_CXXCONF="build-libstdcxx/include/arm-none-eabi/bits/c++config.h"
if ! diff <(grep -E "^#define _GLIBCXX|^/\* #undef _GLIBCXX" "$GEN_CXXCONF" | grep -v "__GLIBCXX__\|_GLIBCXX_HAVE_O_NONBLOCK" | sort) \
          <(grep -E "^#define _GLIBCXX|^/\* #undef _GLIBCXX" "$TC_CXXCONF" | grep -v "__GLIBCXX__\|_GLIBCXX_HAVE_O_NONBLOCK" | sort); then
	echo "ERROR: generated c++config.h differs from the toolchain's (left column is" >&2
	echo "       what this build produced, right is $TC_CXXCONF)." >&2
	echo "       Usually this means the ac_cv_*/glibcxx_cv_* seeds above need" >&2
	echo "       updating for a new libstdc++ probe." >&2
	echo "" >&2
	echo "       But if the toolchain's column claims POSIX facilities that bare" >&2
	echo "       metal cannot have (_GLIBCXX_HAVE_GETENTROPY, _GLIBCXX_HAVE_ARC4RANDOM," >&2
	echo "       _GLIBCXX_USE_FCHMOD, _GLIBCXX_HAVE_SYMLINK, ...), the toolchain's own" >&2
	echo "       libstdc++ was configured with host probe results leaking in -- check" >&2
	echo "       with:" >&2
	echo "         arm-none-eabi-nm -u \$($TC/arm-none-eabi-gcc $ARCH_FLAGS -print-file-name=libstdc++.a) | sort -u" >&2
	echo "       and if that lists getentropy/arc4random/symlink/..., the toolchain's" >&2
	echo "       libstdc++ is already unusable for plugins and matching it would only" >&2
	echo "       reproduce the breakage. Debian/Ubuntu's libstdc++-arm-none-eabi-dev" >&2
	echo "       is known to be built this way; use an ARM GNU toolchain release" >&2
	echo "       (https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)" >&2
	echo "       instead." >&2
	exit 1
fi
echo "c++config.h: OK (identical configuration)"

##############################################################################
echo "==== 4. unwinder + glue objects"
# The ARM EABI unwinder sources come from the gcc tarball's libgcc/ (the
# file list matches what libgcc's t-bpabi puts in LIB2ADDEH for arm-none-eabi
# targets). glue/ carries stubs for headers normally generated by the libgcc
# build system (tconfig.h, tsystem.h, auto-target.h).
PL="$SDK_DIR/plugin-libc"
LIBGCC_SRC="$BASE/$GCC_VER/libgcc"
mkdir -p glue-obj && cd glue-obj
CC="$TC/arm-none-eabi-gcc"
CXX="$TC/arm-none-eabi-g++"
GLUE_INC="-I$PL/glue -I$LIBGCC_SRC"
$CC  $ARCH_FLAGS $PIC_FLAGS $GLUE_INC -fexceptions -fnon-call-exceptions -Wno-address -fvisibility=hidden -c "$PL/glue/unwind-arm.c" -o unwind-arm.o
$CC  $ARCH_FLAGS $PIC_FLAGS $GLUE_INC -c "$LIBGCC_SRC/config/arm/libunwind.S" -o libunwind.o
$CC  $ARCH_FLAGS $PIC_FLAGS $GLUE_INC -fexceptions -fnon-call-exceptions -fvisibility=hidden -c "$LIBGCC_SRC/config/arm/pr-support.c" -o pr-support.o
$CC  $ARCH_FLAGS $PIC_FLAGS $GLUE_INC -fexceptions -fnon-call-exceptions -fvisibility=hidden -c "$LIBGCC_SRC/unwind-c.c" -o unwind-c.o
$CXX $ARCH_FLAGS $PIC_FLAGS -fvisibility=hidden -c "$PL/glue/vterminate_lite.cc" -o vterminate_lite.o
$CC  $ARCH_FLAGS $PIC_FLAGS -c "$PL/dso_handle.c" -o dso_handle.o
# POSIX basename() (libgen.h): lives in newlib's libc/unix/, which ARM's
# newlib configuration does not build, so it never lands in libc.a. Plugins
# use it, so compile it straight from the newlib source tree.
$CC  $ARCH_FLAGS $PIC_FLAGS -c "$BASE/$NEWLIB_VER/newlib/libc/unix/basename.c" -o basename.o
cd "$WORK"

##############################################################################
echo "==== 5. archive surgery"
AR="$TC/arm-none-eabi-ar"
RANLIB="$TC/arm-none-eabi-gcc-ranlib"

cp "$NEWLIB_BUILD/arm-none-eabi/newlib/libc.a" libc-plugin.a
cp "$NEWLIB_BUILD/arm-none-eabi/newlib/libm.a" libm-plugin.a
cp build-libstdcxx/src/.libs/libstdc++.a libstdc++-plugin.a

# Delete archive members by name, erroring if any expected member is absent
# (a rename in a new newlib/libstdc++ version must be noticed, not skipped).
delete_members() {
	local archive="$1"; shift
	local member
	for member in "$@"; do
		if ! $AR t "$archive" | grep -qx "$member"; then
			echo "ERROR: expected member $member not found in $archive -- member list needs updating" >&2
			exit 1
		fi
	done
	$AR d "$archive" "$@"
}

# Symbols that must resolve to the firmware at load time: delete the newlib
# members that define them.
#  - malloc family + abort: firmware's heap/abort
#  - _*_r reentrant syscalls: exported by the firmware (see api-symbols.txt)
#  - init/fini/__call_atexit: the plugin loader runs .init_array itself
#  - stat: the firmware exports stat() but not _stat/_stat_r, so newlib's
#    sysstat.o/statr.o wrapper chain (stat -> _stat_r -> _stat) would shadow
#    the firmware's stat and leave _stat unresolvable
delete_members libc-plugin.a \
	libc_a-sysstat.o libc_a-statr.o \
	libc_a-malloc.o libc_a-mallocr.o libc_a-calloc.o libc_a-callocr.o \
	libc_a-realloc.o libc_a-reallocr.o libc_a-freer.o \
	libc_a-malign.o libc_a-malignr.o libc_a-msize.o \
	libc_a-abort.o \
	libc_a-init.o libc_a-fini.o libc_a-__call_atexit.o \
	libc_a-closer.o libc_a-fstatr.o libc_a-gettimeofdayr.o \
	libc_a-isattyr.o libc_a-lseekr.o libc_a-openr.o libc_a-readr.o \
	libc_a-sbrkr.o libc_a-signalr.o libc_a-timesr.o libc_a-writer.o

# Use the lightweight verbose terminate handler instead of the demangler
delete_members libstdc++-plugin.a vterminate.o cp-demangle.o

##############################################################################
echo "==== 6. merge into $OUT_NAME"
rm -f "$OUT_NAME"
$AR -M <<MRI
CREATE $OUT_NAME
ADDLIB libstdc++-plugin.a
ADDLIB libc-plugin.a
ADDLIB libm-plugin.a
ADDMOD glue-obj/unwind-arm.o
ADDMOD glue-obj/libunwind.o
ADDMOD glue-obj/pr-support.o
ADDMOD glue-obj/unwind-c.o
ADDMOD glue-obj/vterminate_lite.o
ADDMOD glue-obj/dso_handle.o
ADDMOD glue-obj/basename.o
SAVE
END
MRI
"$TC/arm-none-eabi-strip" -g "$OUT_NAME"
$RANLIB "$OUT_NAME"

ls -lh "$WORK/$OUT_NAME"
echo ""
echo "Done. To test it, copy it into plugin-libc/lib/:"
echo "  cp $WORK/$OUT_NAME $SDK_DIR/plugin-libc/lib/$OUT_NAME"
echo "and rebuild a plugin with the gcc $GCC_MAJOR toolchain."
