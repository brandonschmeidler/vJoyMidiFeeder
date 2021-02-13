#include <filesystem>

//#include <cstdlib>
#include "RtMidi.h"

#include <tchar.h>
#include <Windows.h>
#include "public.h"
#include "vjoyinterface.h"

#include <fstream>
#include "nlohmann/json.hpp"
using JSON = nlohmann::json;

const int MIDI_MSG_STS_NOTE_OFF = 128;
const int MIDI_MSG_STS_NOTE_ON = 144;
const int MIDI_MSG_STS_CC = 176;
const int MIDI_MSG_STS_PC = 192;
const std::string APP_PATH_CONFIG = "configs/";

struct VJOY_AXIS_BIND {
	int device; // vJoy device ID
	int axis; // use HID_USAGE_ constants
	int status;
	int value;
};

std::vector<VJOY_AXIS_BIND> axis_bindings;

bool midi_status_is(int status_expected, int status) {
	return status >= status_expected && status < status_expected + 16;
}

void print_note_off_message(std::vector<unsigned char>* message) {
	int channel = (int)message->at(0) - MIDI_MSG_STS_NOTE_OFF + 1;
	int note = (int)message->at(1);
	int velocity = (int)message->at(2);
	printf("NOTE OFF|\tChannel: %d\tNote: %d\tVelocity: %d\n", channel, note, velocity);
}

void print_note_on_message(std::vector<unsigned char>* message) {
	int channel = (int)message->at(0) - MIDI_MSG_STS_NOTE_ON + 1;
	int note = (int)message->at(1);
	int velocity = (int)message->at(2);
	printf("NOTE ON|\tChannel: %d\tNote: %d\tVelocity: %d\n", channel, note, velocity);
}

void print_cc_message(std::vector<unsigned char>* message) {
	int channel = (int)message->at(0) - MIDI_MSG_STS_CC + 1;
	int cc = (int)message->at(1);
	int value = (int)message->at(2);
	printf("CONTROL CHANGE|\tChannel: %d\tCC: %d\tValue: %d\n", channel, cc, value);
}

void print_pc_message(std::vector<unsigned char>* message) {
	int channel = (int)message->at(0) - MIDI_MSG_STS_PC + 1;
	int pc = (int)message->at(1);
	printf("PROGRAM CHANGE|\tChannel: %d\tPC: %d\n", channel, pc);
}

void print_midi_message(std::vector<unsigned char>* message) {
	int status = (int)message->at(0);
	if (midi_status_is(MIDI_MSG_STS_NOTE_OFF, status)) {
		print_note_off_message(message);
	}
	else if (midi_status_is(MIDI_MSG_STS_NOTE_ON, status)) {
		print_note_on_message(message);
	}
	else if (midi_status_is(MIDI_MSG_STS_CC, status)) {
		print_cc_message(message);
	}
	else if (midi_status_is(MIDI_MSG_STS_PC, status)) {
		print_pc_message(message);
	}
}

void midi_callback(double deltatime, std::vector< unsigned char > *message, void *userData)
{
	static JOYSTICK_POSITION_V2 iReport;
	//print_midi_message(message);
	int status = (int)message->at(0);
	int byte1 = (int)message->at(1);
	for (auto& it : axis_bindings) {
		if (status == it.status && byte1 == it.value) {
			// this is a bound midi message
			int byte2 = (int)message->at(2);

			iReport.bDevice = it.device;
			switch (it.axis) {
			case HID_USAGE_X:
				iReport.wAxisX = (byte2 / 127.0f) * 0x8000;
				break;
			case HID_USAGE_Y:
				iReport.wAxisY = (byte2 / 127.0f) * 0x8000;
				break;
			case HID_USAGE_Z:
				iReport.wAxisZ = (byte2 / 127.0f) * 0x8000;
				break;
			case HID_USAGE_RX:
				iReport.wAxisXRot = (byte2 / 127.0f) * 0x8000;
				break;
			case HID_USAGE_RY:
				iReport.wAxisYRot = (byte2 / 127.0f) * 0x8000;
				break;
			case HID_USAGE_RZ:
				iReport.wAxisZRot = (byte2 / 127.0f) * 0x8000;
				break;
			case HID_USAGE_SL0:
				iReport.wSlider = (byte2 / 127.0f) * 0x8000;
				break;
			case HID_USAGE_SL1:
				iReport.wDial = (byte2 / 127.0f) * 0x8000;
				break;
			}
		}
	}

	UpdateVJD(1, (PVOID)(&iReport));
}


int main() {

	if (!std::filesystem::exists(APP_PATH_CONFIG)) {
		std::filesystem::create_directory(APP_PATH_CONFIG);
	}

	
	// Loading bindings
	std::string config_path = "MPKmini2 0.json";
	if (std::filesystem::exists(APP_PATH_CONFIG + config_path)) {
		std::ifstream i(APP_PATH_CONFIG + config_path);
		JSON j;
		i >> j;

		JSON jaxes = j["axes"];
		for (auto&[key, value] : jaxes.items()) {
			//std::cout << key << " : " << value << std::endl;
			VJOY_AXIS_BIND binding;
			binding.device = (int)value["device"];

			std::string saxis = (std::string)value["axis"];
			if (saxis == "X") {
				binding.axis = HID_USAGE_X;
			}
			else if (saxis == "Y") {
				binding.axis = HID_USAGE_Y;
			}
			else if (saxis == "Z") {
				binding.axis = HID_USAGE_Z;
			} else if(saxis == "RX") {
				binding.axis = HID_USAGE_RX;
			}
			else if (saxis == "RY") {
				binding.axis = HID_USAGE_RY;
			}
			else if (saxis == "RZ") {
				binding.axis = HID_USAGE_RZ;
			}
			else if (saxis == "SL0") {
				binding.axis = HID_USAGE_SL0;
			}
			else if (saxis == "SL1") {
				binding.axis = HID_USAGE_SL1;
			}

			//binding.axis = (std::string)value["axis"];
			binding.status = (int)value["status"];
			binding.value = (int)value["value"];
			axis_bindings.push_back(binding);
		}
	}

	// vJoy setup
	if (!vJoyEnabled()) {
		printf("vJoy is disabled\n\n");
		return 1;
	}

	AcquireVJD(1);


	// Setup MIDI and start running 
	RtMidiIn* midi = new RtMidiIn();
	midi->openPort(0);
	printf("%s port opened.\n\n", midi->getPortName(0).c_str());

	midi->setCallback(midi_callback);

	std::cout << "Reading MIDI Input. Press ENTER to quit.\n\n";
	char input;
	std::cin >> input;

	midi->closePort();
	printf("%s port closed.\n\n", midi->getPortName(0).c_str());
	
	RelinquishVJD(1);
	delete midi;
	
	return 0;
}