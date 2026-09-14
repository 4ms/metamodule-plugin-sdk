// Test plugin for .init_array / .fini_array support in the MetaModule plugin
// loader.
//
// __attribute__((constructor)) functions are placed in .init_array, which the
// loader runs when the plugin is loaded (before init()).
// __attribute__((destructor)) functions are placed in .fini_array, which the
// loader runs when the plugin is unloaded, in reverse order.
//
// Expected console output on load:
//   [fini-test] ctor A (priority 101): step 1
//   [fini-test] ctor B (priority 102): step 2
//   [fini-test] init(): ctors ran = 2 PASS
// Expected console output on unload:
//   [fini-test] dtor B (priority 102): step 3 PASS
//   [fini-test] dtor A (priority 101): step 4 PASS
//
// Note: destructors of C++ global objects are not in .fini_array: GCC
// registers them at construction with __aeabi_atexit, so they are not tested
// here.

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"

#include <cstdint>
#include <cstdio>
#include <string_view>

namespace
{

// Counts ctor/dtor executions so each can verify the order it ran in
int step = 0;

// Constructor priorities: lower numbers run first. Destructors with the same
// priorities run in the opposite order: higher numbers run first.
[[gnu::constructor(101)]] void ctor_a() {
	step++;
	printf("[fini-test] ctor A (priority 101): step %d %s\n", step, step == 1 ? "PASS" : "FAIL");
}

[[gnu::constructor(102)]] void ctor_b() {
	step++;
	printf("[fini-test] ctor B (priority 102): step %d %s\n", step, step == 2 ? "PASS" : "FAIL");
}

[[gnu::destructor(102)]] void dtor_b() {
	step++;
	printf("[fini-test] dtor B (priority 102): step %d %s\n", step, step == 3 ? "PASS" : "FAIL");
}

[[gnu::destructor(101)]] void dtor_a() {
	step++;
	printf("[fini-test] dtor A (priority 101): step %d %s\n", step, step == 4 ? "PASS" : "FAIL");
}

class FiniTestCore : public CoreProcessor {
public:
	void update() override {
	}
	void set_samplerate(float) override {
	}
	void set_param(int, float) override {
	}
	void set_input(int, float) override {
	}
	float get_output(int output_id) const override {
		return output_id == 0 ? static_cast<float>(step) : 0.f;
	}
};

struct FiniTestInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"FiniTest"};
	static constexpr std::string_view description{"Init/Fini Array Test"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"FiniArrayTest/panel.png"};
};

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	printf("[fini-test] init(): ctors ran = %d %s\n", step, step == 2 ? "PASS" : "FAIL");
	printf("[fini-test] unload the plugin to run the .fini_array destructors\n");
	MetaModule::register_module<FiniTestCore, FiniTestInfo>("FiniArrayTest");
}
