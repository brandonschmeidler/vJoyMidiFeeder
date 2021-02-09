#pragma once
#include "RtMidi.h"
#include <vector>

class MIDIManager
{
public:
	void add_midi_port();
	void remove_midi_port(int index);

private:
	RtMidiIn m_midi_input_monitor;
	std::vector<RtMidiIn*> m_midi_in_ports;
};

