#pragma once
#include "util/static_string.hh"
#include <cstdint>

namespace MetaModule::System
{

// The USB-C port's current data role combined with the active class. Mirrors the
// firmware's MetaModule::UsbConnection (kept in lockstep by static_asserts in
// coreproc_plugin/system/usb.cc).
//
//   "Host..."   = the MetaModule is the USB host; a peripheral is attached to it.
//   "Device..." = the MetaModule is a USB device plugged into a host (computer).
enum class UsbConnection : uint32_t {
	None,			   // Not attached
	HostSearching,	   // Host: powered, no device class active yet
	HostMidiDevice,	   // Host: a USB-MIDI device is attached
	HostUsbDrive,	   // Host: a USB mass-storage drive is attached
	DeviceWaiting,	   // Device: not yet enumerated by a host
	DeviceMidiHost,	   // Device: enumerated as a USB-MIDI device by a host
	DeviceVideoHost,   // Device: enumerated as a UVC video device by a host
	DeviceConsoleHost, // Device: enumerated as a CDC serial console by a host

	// Forced to device role and idle, but a downstream device (e.g. a USB drive)
	// was sensed on the port -- unusable until the USB Mode is set to Auto or Host.
	DeviceModePeripheralIgnored,
};

// Snapshot of the current USB connection.
//
// The device fields (vid/pid/manufacturer/product/jacks) are populated only when
// the MetaModule is acting as a USB *host* with a peripheral attached. In device
// mode (plugged into a computer) there is no peripheral descriptor to report, so
// those fields are zero/empty and only `connection` is meaningful.
struct UsbConnectionStatus {
	UsbConnection connection = UsbConnection::None;
	uint16_t vid = 0;			   // attached device's idVendor (host mode)
	uint16_t pid = 0;			   // attached device's idProduct (host mode)
	uint8_t num_midi_in_jacks = 0; // MIDI IN jacks declared by the device
	uint8_t num_midi_out_jacks = 0;
	StaticString<63> manufacturer; // iManufacturer string (may be empty)
	StaticString<63> product;	   // iProduct string (may be empty)
};

// Returns the current USB connection status. Cheap; safe to poll. Returns a
// consistent snapshot (the fields never tear relative to each other).
UsbConnectionStatus get_usb_connection_status();

} // namespace MetaModule::System
