#include "AppHeaders.h"

enum eAxis {
	X, Y, Z, RX, RY, RZ, SL0, SL1
};

//bool app_running = true;

//float running_time = 0.0f;
//float closing_time = 10.0f;
//float frame_time = 0.0f;
//const float max_frame_time = 1.0f / 60.0f; // 60fps

const int vjoy_device_id = 1;
const int midi_device_id = 0;
const int MIDI_MSG_CC = 176;
const long max_value = 0x8000;

BOOL used_axis_cache[8];

JOYSTICK_POSITION_V2 iReport;

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

//void update() {
//
//	iReport.bDevice = (BYTE)vjoy_device_id;
//
//	long value = 0;
//
//	if (used_axis_cache[eAxis::X])
//		iReport.wAxisX = value;
//
//	if (used_axis_cache[eAxis::Y])
//		iReport.wAxisY = value;
//
//	if (used_axis_cache[eAxis::Z])
//		iReport.wAxisZ = value;
//
//	if (used_axis_cache[eAxis::RX])
//		iReport.wAxisXRot = value;
//
//	if (used_axis_cache[eAxis::RY])
//		iReport.wAxisYRot = value;
//
//	if (used_axis_cache[eAxis::RZ])
//		iReport.wAxisZRot = value;
//
//	if (used_axis_cache[eAxis::SL0])
//		iReport.wSlider = value;
//
//	if (used_axis_cache[eAxis::SL1])
//		iReport.wDial = value;
//
//	UpdateVJD(vjoy_device_id, (PVOID)(&iReport));
//}

void midi_monitor_callback(double deltatime, std::vector< unsigned char > *message, void *userData)
{
	unsigned int nBytes = message->size();
	for (unsigned int i = 0; i < nBytes; i++)
		std::cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
	if (nBytes > 0)
		std::cout << "stamp = " << deltatime << std::endl;
	
	int status = (int)message->at(0);
	if (status >= MIDI_MSG_CC && status < MIDI_MSG_CC + 16) {
		int channel = status - MIDI_MSG_CC + 1;
		int cc = (int)message->at(1);
		int val = (int)message->at(2);



		iReport.wAxisX = (val / 128.0f) * max_value;

		UpdateVJD(vjoy_device_id, (PVOID)(&iReport));
	}

}

int main() {
	
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
		printf("Vendor: %S\nProduct :%S\nVersion Number:%S\n\n", \
			TEXT(GetvJoyManufacturerString()), \
			TEXT(GetvJoyProductString()), \
			TEXT(GetvJoySerialNumberString()));
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

	//auto time_now = Clock::now();
	//while (app_running) {

	//	auto time_then = time_now;
	//	time_now = Clock::now();

	//	float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(time_now - time_then).count() / 1000000000.0f;
	//	running_time += dt;
	//	
	//	// Update around roughly 60 times per second
	//	frame_time += dt;
	//	if (frame_time >= max_frame_time) {
	//		frame_time -= max_frame_time;
	//		update();
	//	}

	//	if (running_time >= closing_time) { app_running = false; }

	//}
	printf("\nReading MIDI input ... press <enter> to quit.\n");
	char input;
	std::cin.get(input);


	RelinquishVJD(vjoy_device_id);
	delete midi_in;

	return 0;
}