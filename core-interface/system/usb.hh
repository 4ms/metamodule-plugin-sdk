#pragma once
#include "util/static_string.hh"
#include <cstdint>

namespace MetaModule::System
{

// The USB-C port's current data role combined with the active class.
//   "Host..."   = the MetaModule is the USB host; a peripheral is attached to it.
//   "Device..." = the MetaModule is a USB device plugged into a host (computer).
enum class UsbConnectionType : uint16_t {
	None,			   // Not attached
	HostSearching,	   // Host: powered, no device class active yet
	HostMidiDevice,	   // Host: a USB MIDI device is attached
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
// The device fields (vid/pid/manufacturer/product/counts) are populated only when
// the MetaModule is acting as a USB *host* with a peripheral attached. In device
// mode (plugged into a computer) there is no peripheral descriptor to report, so
// those fields are zero/empty and only `connection` is meaningful.
struct UsbConnectionStatus {
	uint16_t vid = 0; // attached device's idVendor (host mode)
	uint16_t pid = 0; // attached device's idProduct (host mode)

	UsbConnectionType connection = UsbConnectionType::None;
	uint8_t num_midi_in_jacks = 0; // MIDI IN jacks declared by the device
	uint8_t num_midi_out_jacks = 0;

	StaticString<63> manufacturer; // iManufacturer string (may be empty)
	StaticString<63> product;	   // iProduct string (may be empty)
};

// Information about a single USB-MIDI jack (a "port" of the attached device).
//
// A USB-MIDI device declares one jack descriptor per port in each direction.
struct UsbMidiJackInfo {
	StaticString<31> name;	  // the jack's iJack string descriptor (may be empty)
	uint8_t jack_id = 0;	  // bJackID: the device's own ID for this jack
	bool is_embedded = false; // true: Embedded jack (a real port); false: External
	bool valid = false;		  // false if `num` was out of range / no such jack
};

// Most jacks the snapshot can hold per direction.
inline constexpr unsigned MaxMidiJacks = 16;

// Returns the current USB connection status
UsbConnectionStatus get_usb_connection_status();

// Returns info about a MIDI IN/OUT jack of the attached device.
// The returned info's `valid` field is false if `num` is out of range.
UsbMidiJackInfo get_usb_midi_in_jack_info(unsigned num);
UsbMidiJackInfo get_usb_midi_out_jack_info(unsigned num);

} // namespace MetaModule::System
