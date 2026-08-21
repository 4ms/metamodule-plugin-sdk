// Provide _memalign_r for plugins.
// We can't use newlib's _memalign_r because it over-allocates
// and trims back in the stlye malloc wants, which would not
// be compatible with the plugin memory arena used in firmware.

#include <stddef.h>

struct _reent;

extern void *memalign(size_t align, size_t size);

void *_memalign_r(struct _reent *reent, size_t align, size_t size) {
	(void)reent;
	return memalign(align, size);
}
