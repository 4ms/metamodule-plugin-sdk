// Test plugin that faults in init(), for the firmware's abort rescue
// (firmware/system/abort_rescue.cc).
//
// init() registers a module and then faults. The firmware should roll back
// to the rescue scope it armed before calling init(), remove the module
// again, and report that the plugin crashed. It must not freeze.
//
// The kind of fault is chosen when the plugin is built (see README.md):
//   data      writes to an address that is not mapped (data abort)
//   prefetch  calls a function at an address that is not mapped (prefetch abort)
//   undef     executes an undefined instruction
//   abort     calls abort()
//
// Expected console output on load (data abort; the others differ in the
// fault's name and details):
//   [init-fault] init(): registering module InitFault
//   [init-fault] init(): faulting now (data)
//   [abort] data abort: pc 0x........ addr 0x00000010 fsr 0x........
//   [rescue] data abort in proc 3: rolling back
//   Unregistering brand 'InitFaultTest' from crashed plugin

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string_view>

namespace
{

constexpr std::string_view fault_type{INIT_FAULT_TYPE};

// Not mapped by the firmware's MMU table (nothing is, below 0x10000000).
// Volatile so the compiler can't tell where the pointers below point to:
// it would replace a write to a known bad address with a trap instruction.
volatile uintptr_t unmapped_address = 0x10;

void fault() {
	if (fault_type == "data") {
		auto *p = reinterpret_cast<volatile uint32_t *>(unmapped_address);
		*p = 0x12345678;
	}

	else if (fault_type == "prefetch")
	{
		auto func = reinterpret_cast<void (*)()>(unmapped_address);
		func();
	}

	else if (fault_type == "undef")
	{
		asm volatile("udf #0");
	}

	else if (fault_type == "abort")
	{
		abort();
	}
}

class InitFaultCore : public CoreProcessor {
public:
	void update() override {
	}
	void set_samplerate(float) override {
	}
	void set_param(int, float) override {
	}
	void set_input(int, float) override {
	}
	float get_output(int) const override {
		return 0.f;
	}
};

struct InitFaultInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"InitFault"};
	static constexpr std::string_view description{"Init Fault (never loads)"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"InitFaultTest/panel.png"};
};

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	// Registered before the fault so the firmware has something to back out
	printf("[init-fault] init(): registering module InitFault\n");
	MetaModule::register_module<InitFaultCore, InitFaultInfo>();

	printf("[init-fault] init(): faulting now (%s)\n", INIT_FAULT_TYPE);
	fault();

	printf("[init-fault] FAIL: init() is still running after the fault\n");
}
