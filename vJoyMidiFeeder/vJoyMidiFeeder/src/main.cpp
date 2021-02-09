#include "AppHeaders.h"



enum eAxis {
	X, Y, Z, RX, RY, RZ, SL0, SL1
};

bool app_running = true;

float running_time = 0.0f;
float closing_time = 10.0f;
float frame_time = 0.0f;

const int device_id = 1;
const float max_frame_time = 1.0f / 60.0f; // 60fps
const long max_value = 0x8000;

BOOL used_axis_cache[8];



JOYSTICK_POSITION_V2 iReport;

void cache_used_axes() {
	used_axis_cache[eAxis::X] = GetVJDAxisExist(device_id, HID_USAGE_X);
	used_axis_cache[eAxis::Y] = GetVJDAxisExist(device_id, HID_USAGE_Y);
	used_axis_cache[eAxis::Z] = GetVJDAxisExist(device_id, HID_USAGE_Z);
	used_axis_cache[eAxis::RX] = GetVJDAxisExist(device_id, HID_USAGE_RX);
	used_axis_cache[eAxis::RY] = GetVJDAxisExist(device_id, HID_USAGE_RY);
	used_axis_cache[eAxis::RZ] = GetVJDAxisExist(device_id, HID_USAGE_RZ);
	used_axis_cache[eAxis::SL0] = GetVJDAxisExist(device_id, HID_USAGE_SL0);
	used_axis_cache[eAxis::SL1] = GetVJDAxisExist(device_id, HID_USAGE_SL1);
}

void update() {
	const float timescale = 1.0f;
	long value = fabsf(sinf(running_time * timescale)) * max_value;

	iReport.bDevice = (BYTE)device_id;
	
	if (used_axis_cache[eAxis::X])
		iReport.wAxisX = value;

	value = fabsf(sinf(running_time * timescale + 0.1)) * max_value;
	if (used_axis_cache[eAxis::Y])
		iReport.wAxisY = value;

	value = fabsf(sinf(running_time * timescale + 0.2)) * max_value;
	if (used_axis_cache[eAxis::Z])
		iReport.wAxisZ = value;

	value = fabsf(sinf(running_time * timescale + 0.3)) * max_value;
	if (used_axis_cache[eAxis::RX])
		iReport.wAxisXRot = value;

	value = fabsf(sinf(running_time * timescale + 0.4)) * max_value;
	if (used_axis_cache[eAxis::RY])
		iReport.wAxisYRot = value;

	value = fabsf(sinf(running_time * timescale + 0.5)) * max_value;
	if (used_axis_cache[eAxis::RZ])
		iReport.wAxisZRot = value;

	value = fabsf(sinf(running_time * timescale + 0.6)) * max_value;
	if (used_axis_cache[eAxis::SL0])
		iReport.wSlider = value;

	value = fabsf(sinf(running_time * timescale + 0.7)) * max_value;
	if (used_axis_cache[eAxis::SL1])
		iReport.wDial = value;

	UpdateVJD(device_id, (PVOID)(&iReport));
}

int main() {
	
	RtMidiIn* midi_in = new RtMidiIn();
	if (midi_in->getPortCount() == 0) {
		printf("No MIDI devices available.\n\n");
		return -1;
	}
	else {
		std::string name = midi_in->getPortName(0);
		printf("Found device: %s\n\n", name.c_str());
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
	if (!AcquireVJD(device_id)) {
		printf("Failed to acquire vJoy Device #%d\n\n", device_id);
		return -1;
	}
	else {
		printf("Acquired vJoy Device #%d\n\n", device_id);
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


	RelinquishVJD(device_id);
	delete midi_in;

	return 0;
}