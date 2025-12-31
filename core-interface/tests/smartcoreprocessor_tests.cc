#include "../CoreModules/SmartCoreProcessor.hh"
#include "CoreModules/elements/element_info.hh"
#include "doctest.h"
#include <string_view>
#include <vector>

using namespace MetaModule;

struct TestInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"Test"};
	static constexpr std::string_view description{""};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view svg_filename{""};

	using enum Coords;

	static constexpr std::array<Element, 7> Elements{{
		MonoLight{},
		JackInput{{{{to_mm<72>(29.28), to_mm<72>(78.14), Center, "Samp 1", ""}}}},
		JackInput{{{{to_mm<72>(29.28), to_mm<72>(124.26), Center, "In 1", ""}}}},
		JackInput{{{{to_mm<72>(29.28), to_mm<72>(168.85), Center, "Samp 2", ""}}}},
		JackInput{{{{to_mm<72>(29.28), to_mm<72>(214.97), Center, "In 2", ""}}}},
		JackOutput{{{{to_mm<72>(29.28), to_mm<72>(264.07), Center, "Out 1", ""}}}},
		JackOutput{{{{to_mm<72>(29.28), to_mm<72>(312.29), Center, "Out 2", ""}}}},
	}};

	enum class Elem {
		Light1,
		Samp1In,
		In1In,
		Samp2In,
		In2In,
		Out1Out,
		Out2Out,
	};
};

struct TestModuleCore : SmartCoreProcessor<TestInfo> {
	void update() override {
		setLED<TestInfo::Elem::Light1>(0xFF1523);
	}

	void update_led(auto x) {
		setLED<TestInfo::Elem::Light1>(x);
	}

	void set_samplerate(float sr) override {
	}
};

TEST_CASE("setLED") {
	TestModuleCore core;

	core.update();
	CHECK(core.get_led_brightness(0) == 0xFF1523);

	core.update_led(0xFFFFFF);
	CHECK(core.get_led_brightness(0) == 0xFFFFFF);

	core.update_led(0xFFFFFE);
	CHECK(core.get_led_brightness(0) == 0xFFFFFE);

	core.update_led(0x818181);
	CHECK(core.get_led_brightness(0) == 0x818181);
}

// TEST_CASE("Encoding") {
// 	TestModuleCore core;

// 	std::vector<uint8_t> raw{1, 2, 3, 4};
// 	std::string encoded = "AQIDBA==";

// 	// Check base64 encoding works:
// 	CHECK(encoded == core.encode(raw));

// 	// Check base64 decoding works:
// 	auto roundtrip_raw = core.decode(encoded);
// 	for (unsigned i = 0; auto c : roundtrip_raw) {
// 		CHECK(c == raw[i++]);
// 	}
// }
