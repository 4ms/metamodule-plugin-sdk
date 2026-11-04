// Test module for the MetaModule MIDI API (MidiInput / MidiOutput).
//
// MIDI in -> CV:
//   A Note On drives Gate Out high (10V) and Note CV Out to the note's pitch,
//   1V/oct with middle C (note 60) at 0V. A Note Off drops the gate.
//   The LED above Gate Out follows the gate, so incoming MIDI can be verified
//   at a glance with nothing patched.
//
// CV -> MIDI out:
//   A rising edge on Gate In sends a Note On. A falling edge sends the matching
//   Note Off. The note number comes from CV In read at the moment the gate goes
//   high (1V/oct, 0V = note 60). With CV In unpatched it defaults to note 60.
//
// CC Out jack:
//   A CC Out jack demonstrates filtering USB MIDI packets by cable number. This
//   is hard-wired to only output CC values from a USB MIDI cable whose name contains
//   "OutEditor". This matches the MCU/HUI setting on the Arturia BeatStepPro, but
//   may be changed to any string that matches your particular MIDI device.
//   The MIDI Device must be plugged in BEFORE the module is contructed.
//   Demonstrating this works: on the BSP, press the top "KNOBS" button to select "CC".
//   Turning a knob does not change the voltage on the CCOut jack. Next, press "KNOBS"
//   to select MCU/HUI. Turning the knobs should change the voltage on the CCOut jack.

#include "CoreModules/CoreProcessor.hh"
#include "CoreModules/elements/element_counter.hh"
#include "CoreModules/elements/element_info.hh"
#include "CoreModules/register_module.hh"
#include "midi/midi_in.hh"
#include "midi/midi_out.hh"
#include "system/usb.hh"

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string_view>

namespace
{

using namespace MetaModule;

constexpr uint8_t CenterNote = 60;

constexpr float note_to_volts(uint8_t note) {
	return (float(note) - float(CenterNote)) / 12.f;
}

constexpr uint8_t volts_to_note(float volts) {
	int note = int(std::lround(volts * 12.f)) + CenterNote;
	return uint8_t(note < 0 ? 0 : note > 127 ? 127 : note);
}

// Gate thresholds, with hysteresis so a noisy or slewed gate can't chatter
// out a stream of Note On/Off pairs.
constexpr float GateOnVolts = 1.0f;
constexpr float GateOffVolts = 0.5f;

constexpr float GateHighOut = 10.f;

constexpr uint8_t MidiChannel = 0; // channel 1
constexpr uint8_t NoteOnVelocity = 100;

struct MidiIoInfo : ModuleInfoBase {
	static constexpr std::string_view slug{"MidiIO"};
	static constexpr std::string_view description{"MIDI to gate/CV and gate/CV to MIDI"};
	static constexpr uint32_t width_hp = 6;
	static constexpr std::string_view png_filename{"MidiTest/panel.png"};

	static constexpr std::array<Element, 6> Elements{{
		MonoLight{{{{15.24f, 26.f, Coords::Center, "MIDI"}, "4ms/comp/led_x.png"}}, Colors565::Green},
		JackOutput{{{{15.24f, 36.f, Coords::Center, "Gate Out"}, "4ms/comp/jack_x.png"}}},
		JackOutput{{{{22.f, 54.f, Coords::Center, "Note CV"}, "4ms/comp/jack_x.png"}}},
		JackOutput{{{{7.f, 54.f, Coords::Center, "CC out"}, "4ms/comp/jack_x.png"}}},
		JackInput{{{{15.24f, 88.f, Coords::Center, "Gate In"}, "4ms/comp/jack_x.png"}}},
		JackInput{{{{15.24f, 106.f, Coords::Center, "CV In"}, "4ms/comp/jack_x.png"}}},
	}};
};

enum Outputs { GateOut, NoteCvOut, CCOut };
enum Inputs { GateIn, CvIn };
enum Lights { MidiLed };

static_assert(ElementCount::count<MidiIoInfo>() ==
			  ElementCount::Counts{.num_params = 0, .num_lights = 1, .num_inputs = 2, .num_outputs = 3});

class MidiIoCore : public CoreProcessor {
public:
	MidiIoCore() {
		set_filter();
	}

	// Deleting the module with the gate still high would leave whatever we're
	// driving holding a note forever.
	~MidiIoCore() override {
		if (last_gate_in_high)
			send(MidiMessage{uint8_t(0x80 | MidiChannel), sent_note, 0});
	}

	void update() override {
		read_midi_in();
		write_midi_out();
	}

	void set_input(int input_id, float val) override {
		if (input_id == GateIn)
			gate_in_volts = val;
		else if (input_id == CvIn)
			cv_in_volts = val;
	}

	float get_output(int output_id) const override {
		switch (output_id) {
			case GateOut:
				return note_held ? GateHighOut : 0.f;
			case NoteCvOut:
				return note_to_volts(held_note);
			case CCOut:
				return cc_val / 12.7f;
			default:
				return 0.f;
		}
	}

	float get_led_brightness(int led_id) const override {
		return (led_id == MidiLed && note_held) ? 1.f : 0.f;
	}

	void mark_input_patched(int input_id) override {
		if (input_id == CvIn)
			cv_in_patched = true;
	}

	void mark_input_unpatched(int input_id) override {
		if (input_id == CvIn) {
			cv_in_patched = false;
			cv_in_volts = 0.f;
		}
	}

	void mark_all_inputs_unpatched() override {
		cv_in_patched = false;
		cv_in_volts = 0.f;
		gate_in_volts = 0.f;
	}

	void set_samplerate(float) override {
	}

	void set_param(int, float) override {
	}

private:
	// MIDI in -> gate + note CV, and the LED.
	void read_midi_in() {
		// Drain the whole queue every update(): it holds 128 messages and
		// overwrites its oldest entry when full.
		MidiMessage msg;
		while (midi_in.pop_message(&msg)) {
			if (msg.is_noteon()) {
				held_note = msg.note();
				note_held = true;

			} else if (msg.is_noteoff()) {
				// Last-note priority: ignore the Note Off for a note that was
				// already superseded by a newer Note On.
				if (note_held && msg.note() == held_note)
					note_held = false;
			} else if (msg.is_cc()) {
				if (filter_cable && msg.usb_hdr.cable_num == *filter_cable)
					cc_val = msg.ccval();
			}
		}
	}

	// Gate + CV -> MIDI out.
	void write_midi_out() {
		bool gate_high = last_gate_in_high ? (gate_in_volts > GateOffVolts) : (gate_in_volts >= GateOnVolts);

		if (gate_high && !last_gate_in_high) {
			sent_note = cv_in_patched ? volts_to_note(cv_in_volts) : CenterNote;
			send(MidiMessage{uint8_t(0x90 | MidiChannel), sent_note, NoteOnVelocity});

		} else if (!gate_high && last_gate_in_high) {
			send(MidiMessage{uint8_t(0x80 | MidiChannel), sent_note, 0});
		}

		last_gate_in_high = gate_high;
	}

	void set_filter() {
		auto status = MetaModule::System::get_usb_connection_status();

		for (unsigned c = 0; c < status.num_midi_rx_cables; c++) {
			auto cable = System::get_usb_midi_rx_cable(c);
			if (cable.name.contains("OutEditor")) {
				filter_cable = c;
				return;
			}
		}

		filter_cable = std::nullopt;
	}

	void send(MidiMessage msg) {
		if (!midi_out.is_queue_full())
			midi_out.push_message(msg);
	}

	MidiInput midi_in;
	MidiOutput midi_out;

	// MIDI in -> CV state
	uint8_t held_note = CenterNote;
	bool note_held = false;

	// CV -> MIDI out state
	float gate_in_volts = 0.f;
	float cv_in_volts = 0.f;
	float cc_val = 0.f;
	bool cv_in_patched = false;
	bool last_gate_in_high = false;
	uint8_t sent_note = CenterNote;

	std::optional<uint8_t> filter_cable{};
};

} // namespace

extern "C" __attribute__((visibility("default"))) void init() {
	printf("[midi-test] plugin loaded\n");
	MetaModule::register_module<MidiIoCore, MidiIoInfo>();
}
