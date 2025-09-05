set(libc ${CMAKE_CURRENT_LIST_DIR}/newlib/libc)

########### argz

set(LIBC_ARGZ_SOURCES
## All functions in this directory are EL/IX level 2 and above
	${libc}/argz/argz_add.c
	${libc}/argz/argz_add_sep.c
	${libc}/argz/argz_append.c
	${libc}/argz/argz_count.c
	${libc}/argz/argz_create.c
	${libc}/argz/argz_create_sep.c
	${libc}/argz/argz_delete.c
	${libc}/argz/argz_extract.c
	${libc}/argz/argz_insert.c
	${libc}/argz/argz_next.c
	${libc}/argz/argz_replace.c
	${libc}/argz/argz_stringify.c
	${libc}/argz/buf_findstr.c
	${libc}/argz/envz_entry.c
	${libc}/argz/envz_get.c
	${libc}/argz/envz_add.c
	${libc}/argz/envz_remove.c
	${libc}/argz/envz_merge.c
	${libc}/argz/envz_strip.c
)
########### ctype

set(LIBC_CTYPE_SOURCES
	${libc}/ctype/ctype_.c
	${libc}/ctype/isalnum.c
	${libc}/ctype/isalpha.c
	${libc}/ctype/iscntrl.c
	${libc}/ctype/isdigit.c
	${libc}/ctype/islower.c
	${libc}/ctype/isupper.c
	${libc}/ctype/isprint.c
	${libc}/ctype/ispunct.c
	${libc}/ctype/isspace.c
	${libc}/ctype/isxdigit.c
	${libc}/ctype/tolower.c
	${libc}/ctype/toupper.c
## The following handles EL/IX level 2 functions
	${libc}/ctype/categories.c
	${libc}/ctype/isalnum_l.c
	${libc}/ctype/isalpha_l.c
	${libc}/ctype/isascii.c
	${libc}/ctype/isascii_l.c
	${libc}/ctype/isblank.c
	${libc}/ctype/isblank_l.c
	${libc}/ctype/iscntrl_l.c
	${libc}/ctype/isdigit_l.c
	${libc}/ctype/islower_l.c
	${libc}/ctype/isupper_l.c
	${libc}/ctype/isprint_l.c
	${libc}/ctype/ispunct_l.c
	${libc}/ctype/isspace_l.c
	${libc}/ctype/iswalnum.c
	${libc}/ctype/iswalnum_l.c
	${libc}/ctype/iswalpha.c
	${libc}/ctype/iswalpha_l.c
	${libc}/ctype/iswblank.c
	${libc}/ctype/iswblank_l.c
	${libc}/ctype/iswcntrl.c
	${libc}/ctype/iswcntrl_l.c
	${libc}/ctype/iswctype.c
	${libc}/ctype/iswctype_l.c
	${libc}/ctype/iswdigit.c
	${libc}/ctype/iswdigit_l.c
	${libc}/ctype/iswgraph.c
	${libc}/ctype/iswgraph_l.c
	${libc}/ctype/iswlower.c
	${libc}/ctype/iswlower_l.c
	${libc}/ctype/iswprint.c
	${libc}/ctype/iswprint_l.c
	${libc}/ctype/iswpunct.c
	${libc}/ctype/iswpunct_l.c
	${libc}/ctype/iswspace.c
	${libc}/ctype/iswspace_l.c
	${libc}/ctype/iswupper.c
	${libc}/ctype/iswupper_l.c
	${libc}/ctype/iswxdigit.c
	${libc}/ctype/iswxdigit_l.c
	${libc}/ctype/isxdigit_l.c
	# ${libc}/ctype/jp2uc.c #not implemented
	${libc}/ctype/toascii.c
	${libc}/ctype/toascii_l.c
	${libc}/ctype/tolower_l.c
	${libc}/ctype/toupper_l.c
	${libc}/ctype/towctrans.c
	${libc}/ctype/towctrans_l.c
	${libc}/ctype/towlower.c
	${libc}/ctype/towlower_l.c
	${libc}/ctype/towupper.c
	${libc}/ctype/towupper_l.c
	${libc}/ctype/wctrans.c
	${libc}/ctype/wctrans_l.c
	${libc}/ctype/wctype.c
	${libc}/ctype/wctype_l.c
)

########### errno
set(LIBC_ERRNO_SOURCES
	${libc}/errno/errno.c
    ${CMAKE_CURRENT_LIST_DIR}/dso_handle.c
)

########### iconv
set(LIBC_ICONV_SOURCES
    #skip
)

########### locale
set(LIBC_LOCALE_SOURCES
	${libc}/locale/setlocale.h
	${libc}/locale/locale.c
	${libc}/locale/localeconv.c
## The following interfaces are EL/IX level 2
	${libc}/locale/duplocale.c
	${libc}/locale/freelocale.c
	${libc}/locale/lctype.c
	${libc}/locale/lmessages.c
	${libc}/locale/lnumeric.c
	${libc}/locale/lmonetary.c
	${libc}/locale/newlocale.c
	${libc}/locale/nl_langinfo.c
	${libc}/locale/timelocal.c
	${libc}/locale/uselocale.c
)

########### machine/arm
# These are removed until we get ASM compilation working
set(LIBC_MACHINE_ARM_SOURCES
	${libc}/machine/arm/setjmp.S 
    ${libc}/machine/arm/strcmp.S 
    ${libc}/machine/arm/strcpy.c
	${libc}/machine/arm/aeabi_memcpy.c 
    ${libc}/machine/arm/aeabi_memcpy-armv7a.S
	${libc}/machine/arm/aeabi_memmove.c 
    ${libc}/machine/arm/aeabi_memmove-soft.S
	${libc}/machine/arm/aeabi_memset.c 
    ${libc}/machine/arm/aeabi_memset-soft.S 
    ${libc}/machine/arm/aeabi_memclr.c
	${libc}/machine/arm/memchr-stub.c
	${libc}/machine/arm/memchr.S
	${libc}/machine/arm/memcpy-stub.c
	${libc}/machine/arm/memcpy.S
	${libc}/machine/arm/strlen-stub.c
	${libc}/machine/arm/strlen.S
)

########### misc
# __dprintf is used for debugging the build
# init is removed because it requires linker symbols that we don't have 
# and we can access the global ctors via linker section .init_array
# fini is removed but it's a question how to access the global dtors? FIXME
set(LIBC_MISC_SOURCES
	# ${libc}/misc/__dprintf.c
	${libc}/misc/ffs.c
	# ${libc}/misc/fini.c
	${libc}/misc/init.c
	${libc}/misc/lock.c
	${libc}/misc/unctrl.c
)

########### posix
#Skip

########### reent
# filesystem (openr, closer,...), system call (signalr), and reent helper (getreent, impure, ...) sources commented out
set(LIBC_REENT_SOURCES
	# ${libc}/reent/closer.c
	# ${libc}/reent/reent.c
	# ${libc}/reent/impure.c
	# ${libc}/reent/fcntlr.c
	# ${libc}/reent/fstatr.c
	# ${libc}/reent/getreent.c
	# ${libc}/reent/gettimeofdayr.c
	# ${libc}/reent/isattyr.c
	# ${libc}/reent/linkr.c
	# ${libc}/reent/lseekr.c
	# ${libc}/reent/mkdirr.c
	# ${libc}/reent/openr.c
	# ${libc}/reent/readr.c
	# ${libc}/reent/renamer.c
	# ${libc}/reent/signalr.c
	# ${libc}/reent/sbrkr.c
	# ${libc}/reent/statr.c
	# ${libc}/reent/timesr.c
	# ${libc}/reent/unlinkr.c
	# ${libc}/reent/writer.c
)

########### search
#Skip

########### signal
#Skip

########### stdio
set(LIBC_STDIO_SOURCES
	${libc}/stdio/fiprintf.c
	${libc}/stdio/fiscanf.c
	${libc}/stdio/iprintf.c
	${libc}/stdio/iscanf.c
	${libc}/stdio/siprintf.c
	${libc}/stdio/siscanf.c
	${libc}/stdio/sniprintf.c
	${libc}/stdio/svfiprintf.c
	${libc}/stdio/svfiscanf.c
	${libc}/stdio/svfprintf.c
	${libc}/stdio/svfscanf.c
	${libc}/stdio/vdiprintf.c
	${libc}/stdio/vfprintf.c
	${libc}/stdio/vfiprintf.c
	${libc}/stdio/vfiscanf.c
	${libc}/stdio/vfscanf.c
	${libc}/stdio/vfwprintf.c
	${libc}/stdio/viprintf.c
	${libc}/stdio/viscanf.c
	${libc}/stdio/vsiprintf.c
	${libc}/stdio/vsiscanf.c
	${libc}/stdio/vsniprintf.c
	${libc}/stdio/clearerr.c
	${libc}/stdio/fclose.c
	${libc}/stdio/fdopen.c
	${libc}/stdio/feof.c
	${libc}/stdio/ferror.c
	${libc}/stdio/fflush.c
	${libc}/stdio/fgetc.c
	${libc}/stdio/fgetpos.c
	${libc}/stdio/fgets.c
	${libc}/stdio/fileno.c
	${libc}/stdio/findfp.c
	${libc}/stdio/flags.c
	${libc}/stdio/fopen.c
	${libc}/stdio/fprintf.c
	${libc}/stdio/fputc.c
	${libc}/stdio/fputs.c
	${libc}/stdio/fread.c
	${libc}/stdio/freopen.c
	${libc}/stdio/fscanf.c
	${libc}/stdio/fseek.c
	${libc}/stdio/fsetpos.c
	${libc}/stdio/ftell.c
	${libc}/stdio/fvwrite.c
	${libc}/stdio/fwalk.c
	${libc}/stdio/fwrite.c
	${libc}/stdio/getc.c
	${libc}/stdio/getchar.c
	${libc}/stdio/getc_u.c
	${libc}/stdio/getchar_u.c
	${libc}/stdio/getdelim.c
	${libc}/stdio/getline.c
	${libc}/stdio/gets.c
	${libc}/stdio/makebuf.c
	${libc}/stdio/perror.c
	${libc}/stdio/printf.c
	${libc}/stdio/putc.c
	${libc}/stdio/putchar.c
	${libc}/stdio/putc_u.c
	${libc}/stdio/putchar_u.c
	${libc}/stdio/puts.c
	${libc}/stdio/refill.c
	${libc}/stdio/remove.c
	${libc}/stdio/rename.c
	${libc}/stdio/rewind.c
	${libc}/stdio/rget.c
	${libc}/stdio/scanf.c
	${libc}/stdio/sccl.c
	${libc}/stdio/setbuf.c
	${libc}/stdio/setbuffer.c
	${libc}/stdio/setlinebuf.c
	${libc}/stdio/setvbuf.c
	${libc}/stdio/snprintf.c
	${libc}/stdio/sprintf.c
	${libc}/stdio/sscanf.c
	${libc}/stdio/stdio.c
	${libc}/stdio/svfiwprintf.c
	${libc}/stdio/svfiwscanf.c
	${libc}/stdio/svfwprintf.c
	${libc}/stdio/svfwscanf.c
	${libc}/stdio/tmpfile.c
	${libc}/stdio/tmpnam.c
	${libc}/stdio/ungetc.c
	${libc}/stdio/vdprintf.c
	${libc}/stdio/vfiwprintf.c
	${libc}/stdio/vfiwscanf.c
	${libc}/stdio/vfwscanf.c
	${libc}/stdio/vprintf.c
	${libc}/stdio/vscanf.c
	${libc}/stdio/vsnprintf.c
	${libc}/stdio/vsprintf.c
	${libc}/stdio/vsscanf.c
	${libc}/stdio/wbuf.c
	${libc}/stdio/wsetup.c
## The following are EL/IX level 2 interfaces
	${libc}/stdio/asprintf.c
	${libc}/stdio/fcloseall.c
	${libc}/stdio/fseeko.c
	${libc}/stdio/ftello.c
	${libc}/stdio/getw.c
	${libc}/stdio/mktemp.c
	${libc}/stdio/putw.c
	${libc}/stdio/vasprintf.c
	${libc}/stdio/asiprintf.c
	${libc}/stdio/vasiprintf.c
## The following are EL/IX level 2 interfaces
	${libc}/stdio/asnprintf.c
	${libc}/stdio/clearerr_u.c
	${libc}/stdio/dprintf.c
	${libc}/stdio/feof_u.c
	${libc}/stdio/ferror_u.c
	${libc}/stdio/fflush_u.c
	${libc}/stdio/fgetc_u.c
	${libc}/stdio/fgets_u.c
	${libc}/stdio/fgetwc.c
	${libc}/stdio/fgetwc_u.c
	${libc}/stdio/fgetws.c
	${libc}/stdio/fgetws_u.c
	${libc}/stdio/fileno_u.c
	${libc}/stdio/fmemopen.c
	${libc}/stdio/fopencookie.c
	${libc}/stdio/fpurge.c
	${libc}/stdio/fputc_u.c
	${libc}/stdio/fputs_u.c
	${libc}/stdio/fputwc.c
	${libc}/stdio/fputwc_u.c
	${libc}/stdio/fputws.c
	${libc}/stdio/fputws_u.c
	${libc}/stdio/fread_u.c
	${libc}/stdio/fsetlocking.c
	${libc}/stdio/funopen.c
	${libc}/stdio/fwide.c
	${libc}/stdio/fwprintf.c
	${libc}/stdio/fwrite_u.c
	${libc}/stdio/fwscanf.c
	${libc}/stdio/getwc.c
	${libc}/stdio/getwc_u.c
	${libc}/stdio/getwchar.c
	${libc}/stdio/getwchar_u.c
	${libc}/stdio/open_memstream.c
	${libc}/stdio/putwc.c
	${libc}/stdio/putwc_u.c
	${libc}/stdio/putwchar.c
	${libc}/stdio/putwchar_u.c
	${libc}/stdio/stdio_ext.c
	${libc}/stdio/swprintf.c
	${libc}/stdio/swscanf.c
	${libc}/stdio/ungetwc.c
	${libc}/stdio/vasnprintf.c
	${libc}/stdio/vswprintf.c
	${libc}/stdio/vswscanf.c
	${libc}/stdio/vwprintf.c
	${libc}/stdio/vwscanf.c
	${libc}/stdio/wprintf.c
	${libc}/stdio/wscanf.c
	${libc}/stdio/asniprintf.c
	${libc}/stdio/diprintf.c
	${libc}/stdio/vasniprintf.c
)


########### stdlib
# malloc-related sources commented out to force use of firmware's malloc
# abort is commented out so that we can override it in firmware
# call_atexit requires fini_array, FIXME: how to access the global dtors?

set(LIBC_STDLIB_SOURCES
	${libc}/stdlib/__adjust.c
	${libc}/stdlib/__atexit.c
	# ${libc}/stdlib/__call_atexit.c
	${libc}/stdlib/__exp10.c
	${libc}/stdlib/__ten_mu.c
	${libc}/stdlib/_Exit.c
	# ${libc}/stdlib/abort.c
	${libc}/stdlib/abs.c
	# ${libc}/stdlib/aligned_alloc.c
	# ${libc}/stdlib/assert.c
	${libc}/stdlib/atexit.c
	${libc}/stdlib/atof.c
	${libc}/stdlib/atoff.c
	${libc}/stdlib/atoi.c
	${libc}/stdlib/atol.c
	# ${libc}/stdlib/calloc.c
	# ${libc}/stdlib/callocr.c
	# ${libc}/stdlib/cfreer.c
	${libc}/stdlib/div.c
	${libc}/stdlib/dtoa.c
	${libc}/stdlib/dtoastub.c
	${libc}/stdlib/environ.c
	${libc}/stdlib/envlock.c
	${libc}/stdlib/eprintf.c
	${libc}/stdlib/exit.c
	# ${libc}/stdlib/freer.c
	${libc}/stdlib/gdtoa-gethex.c
	${libc}/stdlib/gdtoa-hexnan.c
	${libc}/stdlib/getenv.c
	${libc}/stdlib/getenv_r.c
	${libc}/stdlib/imaxabs.c
	${libc}/stdlib/imaxdiv.c
	${libc}/stdlib/itoa.c
	${libc}/stdlib/labs.c
	${libc}/stdlib/ldiv.c
	${libc}/stdlib/ldtoa.c
	${libc}/stdlib/gdtoa-ldtoa.c
	${libc}/stdlib/gdtoa-gdtoa.c
	${libc}/stdlib/gdtoa-dmisc.c
	${libc}/stdlib/gdtoa-gmisc.c
	# ${libc}/stdlib/mallinfor.c
	# ${libc}/stdlib/malloc.c
	# ${libc}/stdlib/mallocr.c
	# ${libc}/stdlib/mallstatsr.c
	${libc}/stdlib/mblen.c
	${libc}/stdlib/mblen_r.c
	${libc}/stdlib/mbstowcs.c
	${libc}/stdlib/mbstowcs_r.c
	${libc}/stdlib/mbtowc.c
	${libc}/stdlib/mbtowc_r.c
	${libc}/stdlib/mlock.c
	${libc}/stdlib/mprec.c
	# ${libc}/stdlib/msizer.c
	${libc}/stdlib/mstats.c
	${libc}/stdlib/on_exit_args.c
	${libc}/stdlib/quick_exit.c
	${libc}/stdlib/rand.c
	${libc}/stdlib/rand_r.c
	${libc}/stdlib/random.c
	# ${libc}/stdlib/realloc.c
	# ${libc}/stdlib/reallocarray.c
	# ${libc}/stdlib/reallocf.c
	# ${libc}/stdlib/reallocr.c
	${libc}/stdlib/sb_charsets.c
	${libc}/stdlib/strtod.c
	${libc}/stdlib/strtoimax.c
	${libc}/stdlib/strtol.c
	${libc}/stdlib/strtoul.c
	${libc}/stdlib/strtoumax.c
	${libc}/stdlib/utoa.c
	${libc}/stdlib/wcstod.c
	${libc}/stdlib/wcstoimax.c
	${libc}/stdlib/wcstol.c
	${libc}/stdlib/wcstoul.c
	${libc}/stdlib/wcstoumax.c
	${libc}/stdlib/wcstombs.c
	${libc}/stdlib/wcstombs_r.c
	${libc}/stdlib/wctomb.c
	${libc}/stdlib/wctomb_r.c
	#
	${libc}/stdlib/arc4random.c
	${libc}/stdlib/arc4random_uniform.c
	${libc}/stdlib/cxa_atexit.c
	${libc}/stdlib/cxa_finalize.c
	${libc}/stdlib/drand48.c
	${libc}/stdlib/ecvtbuf.c
	${libc}/stdlib/efgcvt.c
	${libc}/stdlib/erand48.c
	${libc}/stdlib/jrand48.c
	${libc}/stdlib/lcong48.c
	${libc}/stdlib/lrand48.c
	${libc}/stdlib/mrand48.c
	${libc}/stdlib/msize.c
	${libc}/stdlib/mtrim.c
	${libc}/stdlib/nrand48.c
	${libc}/stdlib/rand48.c
	${libc}/stdlib/seed48.c
	${libc}/stdlib/srand48.c
	${libc}/stdlib/strtoll.c
	${libc}/stdlib/strtoll_r.c
	${libc}/stdlib/strtoull.c
	${libc}/stdlib/strtoull_r.c
	${libc}/stdlib/wcstoll.c
	${libc}/stdlib/wcstoll_r.c
	${libc}/stdlib/wcstoull.c
	${libc}/stdlib/wcstoull_r.c
	${libc}/stdlib/atoll.c
	${libc}/stdlib/llabs.c
	${libc}/stdlib/lldiv.c

)

set(STDLIB_ELIX_2_SOURCES
	${libc}/stdlib/a64l.c
	${libc}/stdlib/btowc.c
	${libc}/stdlib/getopt.c
	${libc}/stdlib/getsubopt.c
	${libc}/stdlib/l64a.c
	${libc}/stdlib/malign.c
	${libc}/stdlib/malignr.c
	# ${libc}/stdlib/malloptr.c
	${libc}/stdlib/mbrlen.c
	${libc}/stdlib/mbrtowc.c
	${libc}/stdlib/mbsinit.c
	${libc}/stdlib/mbsnrtowcs.c
	${libc}/stdlib/mbsrtowcs.c
	${libc}/stdlib/on_exit.c
	# ${libc}/stdlib/pvallocr.c
	# ${libc}/stdlib/valloc.c
	# ${libc}/stdlib/vallocr.c
	${libc}/stdlib/wcrtomb.c
	${libc}/stdlib/wcsnrtombs.c
	${libc}/stdlib/wcsrtombs.c
	${libc}/stdlib/wctob.c
)

set(STDLIB_ELIX_3_SOURCES
	${libc}/stdlib/putenv.c
	${libc}/stdlib/putenv_r.c
	${libc}/stdlib/setenv.c
	${libc}/stdlib/setenv_r.c
)

set (STDLIB_ELIX_4_SOURCES
	${libc}/stdlib/rpmatch.c
	${libc}/stdlib/system.c
)

# if(${ELIX_LEVEL_1})
# set(LIBC_STDLIB_ELIX_SOURCES)
# else()
# if(${ELIX_LEVEL_2})
# set(LIBC_STDLIB_ELIX_SOURCES ${STDLIB_ELIX_2_SOURCES})
# else()
# if(${ELIX_LEVEL_3})
# set(LIBC_STDLIB_ELIX_SOURCES ${STDLIB_ELIX_2_SOURCES} ${STDLIB_ELIX_3_SOURCES})
# else()
set(LIBC_STDLIB_ELIX_SOURCES ${STDLIB_ELIX_2_SOURCES} ${STDLIB_ELIX_3_SOURCES} ${STDLIB_ELIX_4_SOURCES})
# endif()
# endif()
# endif()

set(LIBC_STDLIB_SOURCES ${LIBC_STDLIB_SOURCES} ${LIBC_STDLIB_ELIX_SOURCES})


########### string

set(LIBC_STRING_SOURCES
	${libc}/string/bcopy.c
	${libc}/string/bzero.c
	${libc}/string/explicit_bzero.c
	${libc}/string/ffsl.c
	${libc}/string/ffsll.c
	${libc}/string/fls.c
	${libc}/string/flsl.c
	${libc}/string/flsll.c
	${libc}/string/index.c
	${libc}/string/memchr.c
	${libc}/string/memcmp.c
	${libc}/string/memcpy.c
	${libc}/string/memmove.c
	${libc}/string/memset.c
	${libc}/string/rindex.c
	${libc}/string/strcasecmp.c
	${libc}/string/strcat.c
	${libc}/string/strchr.c
	${libc}/string/strcmp.c
	${libc}/string/strcoll.c
	${libc}/string/strcpy.c
	${libc}/string/strcspn.c
	${libc}/string/strdup.c
	${libc}/string/strdup_r.c
	${libc}/string/strerror.c
	${libc}/string/strerror_r.c
	${libc}/string/strlcat.c
	${libc}/string/strlcpy.c
	${libc}/string/strlen.c 
	${libc}/string/strlwr.c
	${libc}/string/strncasecmp.c
	${libc}/string/strncat.c
	${libc}/string/strncmp.c
	${libc}/string/strncpy.c
	${libc}/string/strnlen.c
	${libc}/string/strnstr.c
	${libc}/string/strpbrk.c
	${libc}/string/strrchr.c
	${libc}/string/strsep.c
	${libc}/string/strsignal.c
	${libc}/string/strspn.c
	${libc}/string/strtok.c
	${libc}/string/strtok_r.c
	${libc}/string/strupr.c
	${libc}/string/strxfrm.c 
	${libc}/string/strstr.c
	${libc}/string/swab.c
	${libc}/string/timingsafe_bcmp.c
	${libc}/string/timingsafe_memcmp.c
	${libc}/string/u_strerr.c
	${libc}/string/wcscat.c
	${libc}/string/wcschr.c
	${libc}/string/wcscmp.c
	${libc}/string/wcscoll.c
	${libc}/string/wcscpy.c
	${libc}/string/wcscspn.c
	${libc}/string/wcslcat.c
	${libc}/string/wcslcpy.c
	${libc}/string/wcslen.c
	${libc}/string/wcsncat.c
	${libc}/string/wcsncmp.c
	${libc}/string/wcsncpy.c
	${libc}/string/wcsnlen.c
	${libc}/string/wcspbrk.c
	${libc}/string/wcsrchr.c
	${libc}/string/wcsspn.c
	${libc}/string/wcsstr.c
	${libc}/string/wcstok.c
	${libc}/string/wcswidth.c
	${libc}/string/wcsxfrm.c
	${libc}/string/wcwidth.c
	${libc}/string/wmemchr.c
	${libc}/string/wmemcmp.c
	${libc}/string/wmemcpy.c
	${libc}/string/wmemmove.c
	${libc}/string/wmemset.c
	${libc}/string/xpg_strerror_r.c
)

# if(!${ELIX_LEVEL_1})
set(LIBC_STRING_SOURCES
    ${LIBC_STRING_SOURCES}
    ${libc}/string/bcmp.c
    ${libc}/string/memccpy.c
    ${libc}/string/mempcpy.c
    ${libc}/string/stpcpy.c
    ${libc}/string/stpncpy.c
    ${libc}/string/strndup.c
    ${libc}/string/strcasestr.c
    ${libc}/string/strchrnul.c
    ${libc}/string/strndup_r.c
    ${libc}/string/wcpcpy.c
    ${libc}/string/wcpncpy.c
    ${libc}/string/wcsdup.c
)
# endif()

# if(!${ELIX_LEVEL_1} AND !${ELIX_LEVEL_2} AND !${ELIX_LEVEL_3})
set(LIBC_STRING_SOURCES
    ${LIBC_STRING_SOURCES}
    # ${libc}/string/gnu_basename.c
    ${libc}/unix/basename.c
    ${libc}/string/memmem.c
    ${libc}/string/memrchr.c
    ${libc}/string/rawmemchr.c
    ${libc}/string/strcasecmp_l.c
    ${libc}/string/strcoll_l.c
    ${libc}/string/strncasecmp_l.c
    ${libc}/string/strverscmp.c
    ${libc}/string/strxfrm_l.c
    ${libc}/string/wcscasecmp.c
    ${libc}/string/wcscasecmp_l.c
    ${libc}/string/wcscoll_l.c
    ${libc}/string/wcsncasecmp.c
    ${libc}/string/wcsncasecmp_l.c
    ${libc}/string/wcsxfrm_l.c
    ${libc}/string/wmempcpy.c
)
# endif()

########### time
set(LIBC_TIME_SOURCES
	${libc}/time/asctime.c
	${libc}/time/asctime_r.c
	${libc}/time/clock.c
	${libc}/time/ctime.c
	${libc}/time/ctime_r.c
	${libc}/time/difftime.c
	${libc}/time/gettzinfo.c
	${libc}/time/gmtime.c
	${libc}/time/gmtime_r.c
	${libc}/time/lcltime.c
	${libc}/time/lcltime_r.c
	${libc}/time/mktime.c
	${libc}/time/month_lengths.c
	${libc}/time/strftime.c  
	${libc}/time/strptime.c
	${libc}/time/time.c
	${libc}/time/tzcalc_limits.c
	${libc}/time/tzlock.c
	${libc}/time/tzset.c
	${libc}/time/tzset_r.c
	${libc}/time/tzvars.c
	${libc}/time/wcsftime.c
)

###########################################################################

target_sources(metamodule-plugin-libc PRIVATE
    ${LIBC_ARGZ_SOURCES}
    ${LIBC_CTYPE_SOURCES}
	${LIBC_ERRNO_SOURCES}
	${LIBC_ICONV_SOURCES}
    ${LIBC_LOCALE_SOURCES}
    ${LIBC_MACHINE_ARM_SOURCES}
    ${LIBC_MISC_SOURCES}
	${LIBC_REENT_SOURCES}
	${LIBC_STRING_SOURCES}
	${LIBC_STDIO_SOURCES}
	${LIBC_STDLIB_SOURCES}
	${LIBC_TIME_SOURCES}
)

target_include_directories(metamodule-plugin-libc PRIVATE
	${libc}/string #for string/local.h to find #include <../ctype/local.h>
)

target_compile_options(metamodule-plugin-libc PRIVATE
    -Wno-sign-compare
    -Wno-maybe-uninitialized
    -Wno-stringop-truncation
    -Wno-unused-but-set-variable
    -Wno-unused-variable
    -Wno-uninitialized
    -Wno-unused-function
    -Wno-unused-label
    -Wno-prio-ctor-dtor #
	"$<$<COMPILE_LANGUAGE:C>:-Wno-pointer-sign>"
	"$<$<COMPILE_LANGUAGE:C>:-Wno-discarded-qualifiers>"
    -I${CMAKE_CURRENT_LIST_DIR}/include
)
set_source_files_properties(
    ${libc}/machine/arm/aeabi_memcpy.c PROPERTIES COMPILE_OPTIONS "-Wno-builtin-declaration-mismatch"
)

target_compile_definitions(metamodule-plugin-libc PRIVATE
    MALLOC_PROVIDED
)

