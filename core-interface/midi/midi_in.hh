#pragma once
#include "midi/midi_router.hh"

namespace MetaModule
{

struct MidiInput {
	MidiQueue queue;

	MidiInput() {
		MidiRouter::subscribe_rx(&queue);
	}

	~MidiInput() {
		MidiRouter::unsubscribe_rx(&queue);
	}

	// Return a the next MIDI message, or std::nullopt if there are no new messages
	std::optional<MidiMessage> pop_message() {
		return queue.get();
	}

	// Same as pop_message() but with a different interface:
	// if there is no message it does nothing and returns false.
	// If there is a message, it returns true and copies the message to
	// the provided message ptr.
	bool pop_message(MidiMessage *message) {
		if (auto next_msg = queue.get(); next_msg.has_value()) {
			if (message != nullptr)
				*message = next_msg.value();
			return true;
		} else {
			return false;
		}
	}
};

} // namespace MetaModule
