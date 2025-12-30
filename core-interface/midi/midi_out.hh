#pragma once
#include "midi/midi_router.hh"

namespace MetaModule
{

struct MidiOutput {
	MidiQueue queue;

	MidiOutput() {
		MidiRouter::subscribe_tx(&queue);
	}

	~MidiOutput() {
		MidiRouter::unsubscribe_tx(&queue);
	}

	void push_message(MidiMessage msg) {
		queue.put(msg);
	}

	bool is_queue_full() const {
		return queue.full();
	}
};

} // namespace MetaModule
