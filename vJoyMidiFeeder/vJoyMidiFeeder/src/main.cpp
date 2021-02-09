#include "AppHeaders.h"
#include <unordered_map>

enum eAxis {
	X, Y, Z, RX, RY, RZ, SL0, SL1
};

struct MIDI_CC {
	int channel;
	int id;

	MIDI_CC() {
		channel = 1;
		id = 0;
	}

	MIDI_CC(int midi_channel, int cc) {
		channel = midi_channel;
		id = cc;
	}
};

struct VJOYMIDI_BINDING {
	int device_id;
	MIDI_CC axes[8];
};

int vjoy_device_id = 1;
int midi_device_id = 0;
const int MIDI_MSG_CC = 176;
const long max_value = 0x8000;

BOOL used_axis_cache[8];

JOYSTICK_POSITION_V2 iReport;
VJOYMIDI_BINDING binding;
eAxis axis_bind_type = eAxis::X;

MIDI_CC last_midi_message;

void cache_used_axes() {
	used_axis_cache[eAxis::X] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_X);
	used_axis_cache[eAxis::Y] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_Y);
	used_axis_cache[eAxis::Z] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_Z);
	used_axis_cache[eAxis::RX] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_RX);
	used_axis_cache[eAxis::RY] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_RY);
	used_axis_cache[eAxis::RZ] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_RZ);
	used_axis_cache[eAxis::SL0] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_SL0);
	used_axis_cache[eAxis::SL1] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_SL1);
}

void midi_bind_callback(double deltatime, std::vector<unsigned char>* message, void* userData) {
	int status = (int)message->at(0);
	if (status >= MIDI_MSG_CC && status < MIDI_MSG_CC + 16) {
		int channel = status - MIDI_MSG_CC + 1;
		int cc = (int)message->at(1);

		MIDI_CC axis = binding.axes[axis_bind_type];
		axis.channel = channel;
		axis.id = cc;
	}
}

void midi_monitor_callback(double deltatime, std::vector< unsigned char > *message, void *userData)
{
	int status = (int)message->at(0);
	if (status >= MIDI_MSG_CC && status < MIDI_MSG_CC + 16) {
		int channel = status - MIDI_MSG_CC + 1;
		int cc = (int)message->at(1);
		int val = (int)message->at(2);

		iReport.bDevice = vjoy_device_id;
		iReport.wAxisX = (val / 128.0f) * max_value;

		UpdateVJD(vjoy_device_id, (PVOID)(&iReport));
	}

}

void clear_console() {
	system("CLS");
	printf("Vendor: %S\nProduct :%S\nVersion Number:%S\n", \
		TEXT(GetvJoyManufacturerString()), \
		TEXT(GetvJoyProductString()), \
		TEXT(GetvJoySerialNumberString()));
	printf("Acquired vJoy Device: %s \t Selected MIDI Device: %s\n", std::to_string(vjoy_device_id).c_str(), std::to_string(midi_device_id).c_str());
}

void show_existing_vjoy_devices() {
	printf("\n\nExisting vJoy Devices (ID value): [ ");
	for (int i = 0; i < 16; ++i) {
		if (isVJDExists(i)) {
			printf((std::string("\t") + std::to_string(i)).c_str());
		}
	}
	printf("\t]\n\n");
}

void show_existing_midi_devices(RtMidiIn* midi_in) {
	printf("\n\nExisting MIDI Devices (ID: Name): \n");
	//unsigned int midi_device_count = midi_in->getPortCount();
	for (unsigned int i = 0; i < midi_in->getPortCount(); ++i) {
		std::string midi_device_name = midi_in->getPortName(i);
		printf((std::to_string(i) + std::string("\t") + midi_device_name + std::string("\n")).c_str());
	}
	printf("\n");
}

int user_select_vjoy_device() {
	show_existing_vjoy_devices();
	printf("Choose a vJoy Device\n");

	int vjoy_select_input;
	std::cin >> vjoy_select_input;
	if (isVJDExists(vjoy_select_input) && GetVJDStatus(vjoy_select_input) == VJD_STAT_FREE) {
		vjoy_device_id = vjoy_select_input;
		clear_console();
		return 0;
	}
	else {
		printf("vJoy Device #%d is disabled.\n", vjoy_select_input);
		return -1;
	}
}

int user_select_midi_device(RtMidiIn* midi_in) {
	show_existing_vjoy_devices();
	printf("Choose a MIDI Device\n");

	int midi_select_input;
	std::cin >> midi_select_input;
	if (midi_select_input >= 0 && midi_select_input < midi_in->getPortCount()) {
		midi_device_id = midi_select_input;
		clear_console();
		return 0;
	}
	else {
		printf("That's not a valid MIDI device\n");
		return -1;
	}
}

int main() {
	
	// Setup midi device
	RtMidiIn* midi_in = new RtMidiIn();
	if (midi_in->getPortCount() == 0) {
		printf("No MIDI devices available.\n\n");
		return -1;
	}
	else {
		std::string name = midi_in->getPortName(midi_device_id);
		printf("Found device: %s\n\n", name.c_str());

		midi_in->openPort(midi_device_id);
		midi_in->setCallback(&midi_monitor_callback);
	}

	// Check that vJoy is running
	if (!vJoyEnabled()) {
		printf("vJoy is disabled. Make sure it's running and click \"Enable vJoy\"\n\n");
		return -1;
	}
	else {
		clear_console();
	}

	// Acquire vJoy device
	if (!AcquireVJD(vjoy_device_id)) {
		printf("Failed to acquire vJoy Device #%d\n\n", vjoy_device_id);
		return -1;
	}
	else {
		printf("Acquired vJoy Device #%d\n\n", vjoy_device_id);
	}

	// Check what axes are used with this vJoy device
	cache_used_axes();

UserInput:
	printf(
		"\n#\tFunction\n1\t%s\n2\t%s\nAwaiting input ... ",
		"Select vJoy Device",
		"Select MIDI Device"
	);
	char input;
	std::cin >> input;

	clear_console();
	switch (input) {
	case '1':
		
		while (user_select_vjoy_device() == -1) {}

		goto UserInput;
		break;
	case '2':
		show_existing_midi_devices(midi_in);

		while (user_select_midi_device(midi_in) == -1) {}

		goto UserInput;
		break;
	default:
		printf("Good bye");
		goto Cleanup;
		break;
	}
	

	// Cleanup 
Cleanup:
	RelinquishVJD(vjoy_device_id);
	delete midi_in;

	// Get outta heaaaa!
	return 0;
}