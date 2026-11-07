// Test plugin for the MetaModule::System USB query API:
//   get_usb_connection_status()
//   get_usb_device_name()
//   get_usb_midi_in_jack_info(n) / get_usb_midi_out_jack_info(n)
//   get_usb_midi_rx_cable(n) / get_usb_midi_tx_cable(n)
//
// Dumps the full connection info to the console at plugin load, at module
// creation, and then once per second whenever anything changes
//
// It's very likely that this module spikes the CPU, so set block size to 512 and SR to 24k.

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"
#include "system/usb.hh"

#include <cstdio>
#include <string_view>

namespace
{

using namespace MetaModule::System;

char const *to_string(UsbConnectionType t) {
	switch (t) {
		case UsbConnectionType::None:
			return "None (not attached)";
		case UsbConnectionType::HostSearching:
			return "HostSearching (host, no class active yet)";
		case UsbConnectionType::HostMidiDevice:
			return "HostMidiDevice (host, USB-MIDI device attached)";
		case UsbConnectionType::HostUsbDrive:
			return "HostUsbDrive (host, mass-storage drive attached)";
		case UsbConnectionType::DeviceWaiting:
			return "DeviceWaiting (device, not yet enumerated)";
		case UsbConnectionType::DeviceMidiHost:
			return "DeviceMidiHost (device, enumerated as USB-MIDI)";
		case UsbConnectionType::DeviceVideoHost:
			return "DeviceVideoHost (device, enumerated as UVC video)";
		case UsbConnectionType::DeviceConsoleHost:
			return "DeviceConsoleHost (device, enumerated as CDC console)";
		case UsbConnectionType::DeviceModePeripheralIgnored:
			return "DeviceModePeripheralIgnored (device mode, peripheral sensed but unusable)";
	}
	return "<unknown enum value>";
}

// Everything the API can report, in a form that's cheap to compare so the
// module can print only when the state actually changes.
struct Snapshot {
	UsbConnectionStatus status;
	UsbDeviceName name;
	UsbMidiJackInfo in_jacks[MaxMidiJacks];
	UsbMidiJackInfo out_jacks[MaxMidiJacks];
	UsbMidiCableInfo rx_cables[MaxMidiCables];
	UsbMidiCableInfo tx_cables[MaxMidiCables];
};

Snapshot take_snapshot() {
	Snapshot s;
	s.status = get_usb_connection_status();
	s.name = get_usb_device_name();

	for (unsigned i = 0; i < MaxMidiJacks; i++) {
		s.in_jacks[i] = get_usb_midi_in_jack_info(i);
		s.out_jacks[i] = get_usb_midi_out_jack_info(i);
	}
	for (unsigned i = 0; i < MaxMidiCables; i++) {
		s.rx_cables[i] = get_usb_midi_rx_cable(i);
		s.tx_cables[i] = get_usb_midi_tx_cable(i);
	}
	return s;
}

// Compares only the fields the console dump shows, so a change in any of them
// re-triggers a dump.
bool differs(Snapshot const &a, Snapshot const &b) {

	if (a.status.connection != b.status.connection || a.status.vid != b.status.vid || a.status.pid != b.status.pid ||
		a.status.num_midi_in_jacks != b.status.num_midi_in_jacks ||
		a.status.num_midi_out_jacks != b.status.num_midi_out_jacks ||
		a.status.num_midi_rx_cables != b.status.num_midi_rx_cables ||
		a.status.num_midi_tx_cables != b.status.num_midi_tx_cables || a.name.manufacturer != b.name.manufacturer ||
		a.name.product != b.name.product)
		return true;

	auto jack_differs = [](UsbMidiJackInfo const &x, UsbMidiJackInfo const &y) {
		return x.valid != y.valid || x.jack_id != y.jack_id || x.is_embedded != y.is_embedded ||
			   x.has_cable != y.has_cable || (x.has_cable && x.cable_num != y.cable_num) || x.name != y.name;
	};

	for (unsigned i = 0; i < MaxMidiJacks; i++) {
		if (jack_differs(a.in_jacks[i], b.in_jacks[i]) || jack_differs(a.out_jacks[i], b.out_jacks[i]))
			return true;
	}

	auto cable_differs = [](UsbMidiCableInfo const &x, UsbMidiCableInfo const &y) {
		return x.valid != y.valid || x.cable_num != y.cable_num || x.name != y.name;
	};

	for (unsigned i = 0; i < MaxMidiCables; i++) {
		if (cable_differs(a.rx_cables[i], b.rx_cables[i]) || cable_differs(a.tx_cables[i], b.tx_cables[i]))
			return true;
	}
	return false;
}

void dump_jacks(char const *dir, UsbMidiJackInfo const *jacks, uint8_t declared) {
	printf("[usb-test]   MIDI %s jacks: %u declared\n", dir, declared);

	for (unsigned i = 0; i < MaxMidiJacks; i++) {
		auto const &j = jacks[i];
		if (!j.valid)
			continue;
		char cable[16];
		if (j.has_cable)
			snprintf(cable, sizeof(cable), "cable=%-2u", j.cable_num);
		else
			snprintf(cable, sizeof(cable), "cable=- ");

		printf("[usb-test]     [%u] jack_id=%-3u %-8s %s name=\"%s\"\n",
			   i,
			   j.jack_id,
			   j.is_embedded ? "Embedded" : "External",
			   cable,
			   j.name.c_str());

		// Only Embedded jacks are reachable over USB, so only they can have a
		// cable number.
		if (j.has_cable && !j.is_embedded)
			printf("[usb-test]     WARNING: External jack %u reports a cable number\n", i);
	}

	// Consistency check: the status struct's count should match the number of
	// jacks that report valid, and jacks past the count must be invalid.
	unsigned num_valid = 0;
	for (unsigned i = 0; i < MaxMidiJacks; i++)
		if (jacks[i].valid)
			num_valid++;

	if (num_valid != declared)
		printf("[usb-test]     WARNING: %u jacks report valid but status says %u\n", num_valid, declared);

	for (unsigned i = declared; i < MaxMidiJacks; i++)
		if (jacks[i].valid)
			printf("[usb-test]     WARNING: jack %u past the declared count reports valid\n", i);
}

// The cable view: what a module would show the user in a "pick a MIDI port" list.
void dump_cables(char const *dir, UsbMidiCableInfo const *cables, uint8_t declared) {
	printf("[usb-test]   MIDI %s cables: %u declared\n", dir, declared);

	for (unsigned c = 0; c < MaxMidiCables; c++) {
		auto const &j = cables[c];
		if (!j.valid) {
			if (c < declared)
				printf("[usb-test]     WARNING: cable %u is within the declared count but is invalid\n", c);
			continue;
		}

		printf("[usb-test]     cable %u: name=\"%s\"\n", c, j.name.c_str());

		if (c >= declared)
			printf("[usb-test]     WARNING: cable %u past the declared count reports valid\n", c);
	}
}

void dump(Snapshot const &s, char const *reason) {
	auto const &st = s.status;
	auto const &n = s.name;

	printf("[usb-test] ---- USB connection info (%s) ----\n", reason);
	printf("[usb-test]   connection: %s (enum %u)\n", to_string(st.connection), static_cast<unsigned>(st.connection));
	printf("[usb-test]   vid:pid: %04x:%04x\n", st.vid, st.pid);
	printf("[usb-test]   manufacturer: \"%s\"\n", n.manufacturer.c_str());
	printf("[usb-test]   product: \"%s\"\n", n.product.c_str());

	dump_jacks("IN ", s.in_jacks, st.num_midi_in_jacks);
	dump_jacks("OUT", s.out_jacks, st.num_midi_out_jacks);

	// rx cables carry messages from the device to us (they're served by the
	// device's Embedded MIDI OUT jacks); tx cables are the other direction.
	dump_cables("RX", s.rx_cables, st.num_midi_rx_cables);
	dump_cables("TX", s.tx_cables, st.num_midi_tx_cables);

	// Out-of-range queries must come back invalid rather than crashing or
	// returning stale data.
	auto past_in = get_usb_midi_in_jack_info(MaxMidiJacks);
	auto past_out = get_usb_midi_out_jack_info(MaxMidiJacks + 100);
	printf("[usb-test]   out-of-range jack query: %s\n",
		   (!past_in.valid && !past_out.valid) ? "PASS (both invalid)" : "FAIL (reported valid)");

	auto past_rx = get_usb_midi_rx_cable(MaxMidiCables);
	auto past_tx = get_usb_midi_tx_cable(MaxMidiCables + 100);
	printf("[usb-test]   out-of-range cable query: %s\n",
		   (!past_rx.valid && !past_tx.valid) ? "PASS (both invalid)" : "FAIL (reported valid)");

	printf("[usb-test] ----------------------------------\n");
}

class UsbInfoCore : public CoreProcessor {
public:
	UsbInfoCore() {
		last = take_snapshot();
		dump(last, "module created");
	}

	void update() override {
		if (++counter < poll_interval)
			return;
		counter = 0;

		auto now = take_snapshot();
		if (differs(last, now)) {
			last = now;
			dump(last, "changed");
		}
	}

	void set_samplerate(float sr) override {
		// Poll about once a second
		poll_interval = (sr > 1.f) ? static_cast<uint32_t>(sr) : 48000u;
	}

	void set_param(int, float) override {
	}

	void set_input(int, float) override {
	}

	float get_output(int output_id) const override {
		switch (output_id) {
			case 0:
				return static_cast<float>(last.status.connection);
			case 1:
				return last.status.connection == UsbConnectionType::None ? 0.f : 10.f;
			case 2:
				return static_cast<float>(last.status.num_midi_in_jacks);
			case 3:
				return static_cast<float>(last.status.num_midi_out_jacks);
			default:
				return 0.f;
		}
	}

private:
	Snapshot last;
	uint32_t counter = 0;
	uint32_t poll_interval = 48000;
};

struct UsbInfoInfo : MetaModule::ModuleInfoBase {
	static constexpr std::string_view slug{"UsbInfo"};
	static constexpr std::string_view description{"USB Connection Info Test"};
	static constexpr uint32_t width_hp = 4;
	static constexpr std::string_view png_filename{"UsbInfoTest/panel.png"};
};

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	printf("[usb-test] plugin loaded\n");
	dump(take_snapshot(), "plugin load");
	MetaModule::register_module<UsbInfoCore, UsbInfoInfo>();
}
