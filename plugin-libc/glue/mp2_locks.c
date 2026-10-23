// newlib's static locks, for MP2 plugins. make_plugin_libc_mp2.cmake removes the toolchain libc's
// lock.o, which defines these and the lock functions (which do nothing). The plugin then imports
// the lock functions from the firmware (system/lock_ca35.cc), so plugin code running on both of
// the A35's cores at once can't corrupt the plugin's stdio state.
//
// The firmware's struct __lock: zeroed is unlocked.
struct __lock {
	unsigned int owner;
	unsigned int count;
	unsigned int masks_irqs;
	unsigned int reserved;
	unsigned long long saved_daif;
};

_Static_assert(sizeof(struct __lock) == 24, "Must match the firmware's struct __lock (system/lock_ca35.cc)");

#define PLUGIN_LOCK(name) __attribute__((visibility("hidden"))) struct __lock __lock_##name

PLUGIN_LOCK(__sfp_recursive_mutex);
PLUGIN_LOCK(__atexit_recursive_mutex);
PLUGIN_LOCK(__malloc_recursive_mutex);
PLUGIN_LOCK(__env_recursive_mutex);
PLUGIN_LOCK(__tz_mutex);
PLUGIN_LOCK(__at_quick_exit_mutex);
PLUGIN_LOCK(__dd_hash_mutex);
PLUGIN_LOCK(__arc4random_mutex);
