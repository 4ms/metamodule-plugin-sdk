// MP2 plugins: libc pieces the toolchain's libc refers to but doesn't define (see
// cmake/make_plugin_libc_mp2.cmake). MP1 plugins get these from the prebuilt plugin-libc archive.

#include <string.h>

// newlib's __libc_fini_array() calls _fini (normally from crti.o). Plugins never run it: the
// plugin loader runs the plugin's .fini_array itself.
__attribute__((visibility("hidden"))) void _fini(void) {
}

// POSIX basename() (newlib's libc.a has no libgen): the last component of path, ignoring trailing
// '/'s (which it removes). "." for an empty path or NULL, "/" for "/".
__attribute__((visibility("hidden"))) char *basename(char *path) {
	static char dot[] = ".";
	if (path == NULL || *path == '\0')
		return dot;

	char *end = path + strlen(path) - 1;
	while (end > path && *end == '/')
		*end-- = '\0';

	char *start = end;
	while (start > path && start[-1] != '/')
		start--;
	return start;
}
