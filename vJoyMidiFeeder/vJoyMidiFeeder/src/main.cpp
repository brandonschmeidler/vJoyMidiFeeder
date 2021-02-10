#include <string>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <tchar.h>
#include <Windows.h>
#include "public.h"
#include "vjoyinterface.h"

#include "RtMidi.h"

#pragma region ImGui Functions
void imgui_new_frame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
#pragma endregion

#pragma region GLFW Functions
static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
#pragma endregion

#pragma region Application Variables
const std::string APP_TITLE = "vJoy MIDI Feeder";
const std::string APP_VERSION = "0.0.0";
const char* GLSL_VERSION = "#version 440";

int selected_vjoy_device = 1;
#pragma endregion

#pragma region MIDI Variables

#pragma endregion

#pragma region vJoy Variables
std::vector<int> get_available_vjoy_devices() {
	std::vector<int> devices;

	for (int i = 0; i < 16; ++i) {
		if (isVJDExists(i)) {
			devices.push_back(i);
		}
	}

	return devices;
}
#pragma endregion


#pragma region Application GUI

void gui_dock_initial_layout(GLFWwindow* window, ImGuiID dockspace_id) {
	static bool ran_already = false;

	if (ran_already) return;

	int width, height;
	glfwGetWindowSize(window, &width, &height);

	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(width, height));

	ImGuiID dock_main_id = dockspace_id;
	
	ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
	ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.20f, nullptr, &dock_main_id);
	
	ImGuiDockNode* dock_node = ImGui::DockBuilderGetNode(dock_id_left);
	dock_node->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;

	dock_node = ImGui::DockBuilderGetNode(dock_id_bottom);
	dock_node->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;

	dock_node = ImGui::DockBuilderGetNode(dock_main_id);
	dock_node->LocalFlags |= ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_NoTabBar;

	ImGui::DockBuilderDockWindow("Device Manager", dock_id_left);
	ImGui::DockBuilderDockWindow("MIDI Message Monitor", dock_id_bottom);
	ImGui::DockBuilderDockWindow("vJoy Device Monitor", dock_main_id);
	ImGui::DockBuilderFinish(dockspace_id);
	ran_already = true;
}

bool gui_tree_bind_axis(const char* label, int channel, int cc) {
	bool ret = false;
	if (ImGui::TreeNode(label)) {
		ImGui::Text("Channel: %d\tCC: %d", channel, cc);
		ImGui::SameLine();
		ret = ImGui::Button((std::string("Bind##") + std::string(label)).c_str());
		ImGui::TreePop();
	}
	return ret;
}

void gui_vjoy_device_monitor() {
	ImGui::Begin("vJoy Device Monitor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

	if (ImGui::TreeNode("Bindings")) {

		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_X) > 0) {
			if (gui_tree_bind_axis("Axis X", 1, 86)) {
				printf("Binding X-Axis");
			}
		}

		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_Y) > 0) {
			if (gui_tree_bind_axis("Axis Y", 1, 86)) {
				printf("Binding Y-Axis");
			}
		}
		
		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_Z) > 0) {
			if (gui_tree_bind_axis("Axis Z", 1, 86)) {
				printf("Binding Z-Axis");
			}
		}
		
		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_RX) > 0) {
			if (gui_tree_bind_axis("Axis RX", 1, 86)) {
				printf("Binding RX-Axis");
			}
		}

		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_RY) > 0) {
			if (gui_tree_bind_axis("Axis RY", 1, 86)) {
				printf("Binding RY-Axis");
			}
		}

		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_RZ) > 0) {
			if (gui_tree_bind_axis("Axis RZ", 1, 86)) {
				printf("Binding RZ-Axis");
			}
		}

		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_SL0) > 0) {
			if (gui_tree_bind_axis("Axis SL0", 1, 86)) {
				printf("Binding SL0-Axis");
			}
		}

		if (GetVJDAxisExist(selected_vjoy_device, HID_USAGE_SL1) > 0) {
			if (gui_tree_bind_axis("Axis SL1", 1, 86)) {
				printf("Binding SL1-Axis");
			}
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void gui_devices_midi() {
	std::vector<std::string> fake_midi_devices;

	for (int i = 0; i < 5; ++i) fake_midi_devices.push_back(std::string("MIDI ") + std::to_string(i));
	static int selected_midi_id = 0;
	ImGui::Text("MIDI Devices");
	ImGui::SameLine();
	ImGui::Button("Refresh##midi_devices");

	ImVec2 region = ImGui::GetContentRegionAvail();
	ImGui::PushItemWidth(region.x);

	if (ImGui::BeginCombo("##combo_MIDIDevices", fake_midi_devices[selected_midi_id].c_str())) {

		for (int i = 0; i < 5; ++i) {
			bool is_selected = selected_midi_id == i;
			if (ImGui::Selectable(fake_midi_devices[i].c_str(), is_selected)) {
				selected_midi_id = i;
				break;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::PopItemWidth();
}

void gui_devices_vjoy() {
	std::vector<int> vjoy_devices = get_available_vjoy_devices();

	ImGui::Text("vJoy Devices");

	ImVec2 region = ImGui::GetContentRegionAvail();
	ImGui::PushItemWidth(region.x);

	static int combo_index = 0;
	if (ImGui::BeginCombo("##combo_vJoyDevices", std::to_string(vjoy_devices[combo_index]).c_str())) {

		int count = vjoy_devices.size();
		for (int i = 0; i < count; ++i) {
			bool is_selected = combo_index == i;
			if (ImGui::Selectable(std::to_string(vjoy_devices[i]).c_str(), is_selected)) {
				combo_index = i;
				selected_vjoy_device = vjoy_devices[i];
				break;
			}
		}
		ImGui::EndCombo();
	}
	
	ImGui::PopItemWidth();
}

void gui_device_manager() {
	ImGui::Begin("Device Manager", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

	gui_devices_midi();
	gui_devices_vjoy();

	ImGui::End();
}

void gui_midi_monitor() {
	ImGui::Begin("MIDI Message Monitor", nullptr, ImGuiWindowFlags_NoMove);
	for (int i = 0; i < 10; ++i) {
		ImGui::Text("Channel: %d\tCC: %d\tValue:%d", 1, i + 25, i * 10);
	}
	ImGui::End();
}

#pragma endregion


int main() {

#pragma region vJoy Setup
	if (!vJoyEnabled()) {
		printf("vJoy is disabled.\n\n");
		return 1;
	}
#pragma endregion

#pragma region Window/OpenGL Setup
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, (APP_TITLE + std::string("\tv") + APP_VERSION).c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#pragma endregion

#pragma region GLFW Callbacks
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
#pragma endregion

#pragma region ImGui Setup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Control
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(GLSL_VERSION);
#pragma endregion

#pragma region Window Loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		imgui_new_frame();
		
		ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		gui_dock_initial_layout(window,dockspace_id);

		gui_vjoy_device_monitor();
		gui_device_manager();
		gui_midi_monitor();

		ImGui::Render();

		glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Let ImGui update viewports
		GLFWwindow* backup_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_context);

		glfwSwapBuffers(window);
	}
#pragma endregion

#pragma region Cleanup

#pragma endregion

	return 0;
}




//#include "nlohmann/json.hpp"
//#include <fstream>
//
//using JSON = nlohmann::json;
//JSON file_to_json(const char* path) {
//	std::ifstream i(path);
//	JSON j;
//	i >> j;
//	return j;
//}
//
//const char* config_path = "C:/Users/brand/Downloads/config.json";
//
//int main() {
//	JSON j = file_to_json(config_path);
//
//
//
//	return 0;
//}

//#include "AppHeaders.h"
//#include <unordered_map>
//
//enum eAxis {
//	X, Y, Z, RX, RY, RZ, SL0, SL1
//};
//
//struct MIDI_CC {
//	int channel;
//	int id;
//
//	MIDI_CC() {
//		channel = 1;
//		id = 0;
//	}
//
//	MIDI_CC(int midi_channel, int cc) {
//		channel = midi_channel;
//		id = cc;
//	}
//};
//
//struct VJOYMIDI_BINDING {
//	int device_id;
//	MIDI_CC axes[8];
//};
//
//int vjoy_device_id = 1;
//int midi_device_id = 0;
//const int MIDI_MSG_CC = 176;
//const long max_value = 0x8000;
//
//BOOL used_axis_cache[8];
//
//JOYSTICK_POSITION_V2 iReport;
//VJOYMIDI_BINDING binding;
//eAxis axis_bind_type = eAxis::X;
//
//MIDI_CC last_midi_message;
//
//void cache_used_axes() {
//	used_axis_cache[eAxis::X] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_X);
//	used_axis_cache[eAxis::Y] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_Y);
//	used_axis_cache[eAxis::Z] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_Z);
//	used_axis_cache[eAxis::RX] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_RX);
//	used_axis_cache[eAxis::RY] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_RY);
//	used_axis_cache[eAxis::RZ] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_RZ);
//	used_axis_cache[eAxis::SL0] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_SL0);
//	used_axis_cache[eAxis::SL1] = GetVJDAxisExist(vjoy_device_id, HID_USAGE_SL1);
//}
//
//void midi_bind_callback(double deltatime, std::vector<unsigned char>* message, void* userData) {
//	int status = (int)message->at(0);
//	if (status >= MIDI_MSG_CC && status < MIDI_MSG_CC + 16) {
//		int channel = status - MIDI_MSG_CC + 1;
//		int cc = (int)message->at(1);
//
//		MIDI_CC axis = binding.axes[axis_bind_type];
//		axis.channel = channel;
//		axis.id = cc;
//	}
//}
//
//void midi_monitor_callback(double deltatime, std::vector< unsigned char > *message, void *userData)
//{
//	int status = (int)message->at(0);
//	if (status >= MIDI_MSG_CC && status < MIDI_MSG_CC + 16) {
//		int channel = status - MIDI_MSG_CC + 1;
//		int cc = (int)message->at(1);
//		int val = (int)message->at(2);
//
//		iReport.bDevice = vjoy_device_id;
//		iReport.wAxisX = (val / 128.0f) * max_value;
//
//		UpdateVJD(vjoy_device_id, (PVOID)(&iReport));
//	}
//
//}
//
//void clear_console() {
//	system("CLS");
//	printf("Vendor: %S\nProduct :%S\nVersion Number:%S\n", \
//		TEXT(GetvJoyManufacturerString()), \
//		TEXT(GetvJoyProductString()), \
//		TEXT(GetvJoySerialNumberString()));
//	printf("Acquired vJoy Device: %s \t Selected MIDI Device: %s\n", std::to_string(vjoy_device_id).c_str(), std::to_string(midi_device_id).c_str());
//}
//
//void show_existing_vjoy_devices() {
//	printf("\n\nExisting vJoy Devices (ID value): [ ");
//	for (int i = 0; i < 16; ++i) {
//		if (isVJDExists(i)) {
//			printf((std::string("\t") + std::to_string(i)).c_str());
//		}
//	}
//	printf("\t]\n\n");
//}
//
//void show_existing_midi_devices(RtMidiIn* midi_in) {
//	printf("\n\nExisting MIDI Devices (ID: Name): \n");
//	//unsigned int midi_device_count = midi_in->getPortCount();
//	for (unsigned int i = 0; i < midi_in->getPortCount(); ++i) {
//		std::string midi_device_name = midi_in->getPortName(i);
//		printf((std::to_string(i) + std::string("\t") + midi_device_name + std::string("\n")).c_str());
//	}
//	printf("\n");
//}
//
//int user_select_vjoy_device() {
//	show_existing_vjoy_devices();
//	printf("Choose a vJoy Device\n");
//
//	int vjoy_select_input;
//	std::cin >> vjoy_select_input;
//	if (isVJDExists(vjoy_select_input) && GetVJDStatus(vjoy_select_input) == VJD_STAT_FREE) {
//		vjoy_device_id = vjoy_select_input;
//		clear_console();
//		return 0;
//	}
//	else {
//		printf("vJoy Device #%d is disabled.\n", vjoy_select_input);
//		return -1;
//	}
//}
//
//int user_select_midi_device(RtMidiIn* midi_in) {
//	show_existing_vjoy_devices();
//	printf("Choose a MIDI Device\n");
//
//	int midi_select_input;
//	std::cin >> midi_select_input;
//	if (midi_select_input >= 0 && midi_select_input < midi_in->getPortCount()) {
//		midi_device_id = midi_select_input;
//		clear_console();
//		return 0;
//	}
//	else {
//		printf("That's not a valid MIDI device\n");
//		return -1;
//	}
//}
//
//int main() {
//	
//	// Setup midi device
//	RtMidiIn* midi_in = new RtMidiIn();
//	if (midi_in->getPortCount() == 0) {
//		printf("No MIDI devices available.\n\n");
//		return -1;
//	}
//	else {
//		std::string name = midi_in->getPortName(midi_device_id);
//		printf("Found device: %s\n\n", name.c_str());
//
//		midi_in->openPort(midi_device_id);
//		midi_in->setCallback(&midi_monitor_callback);
//	}
//
//	// Check that vJoy is running
//	if (!vJoyEnabled()) {
//		printf("vJoy is disabled. Make sure it's running and click \"Enable vJoy\"\n\n");
//		return -1;
//	}
//	else {
//		clear_console();
//	}
//
//	// Acquire vJoy device
//	if (!AcquireVJD(vjoy_device_id)) {
//		printf("Failed to acquire vJoy Device #%d\n\n", vjoy_device_id);
//		return -1;
//	}
//	else {
//		printf("Acquired vJoy Device #%d\n\n", vjoy_device_id);
//	}
//
//	// Check what axes are used with this vJoy device
//	cache_used_axes();
//
//UserInput:
//	printf(
//		"\n#\tFunction\n1\t%s\n2\t%s\nAwaiting input ... ",
//		"Select vJoy Device",
//		"Select MIDI Device"
//	);
//	char input;
//	std::cin >> input;
//
//	clear_console();
//	switch (input) {
//	case '1':
//		
//		while (user_select_vjoy_device() == -1) {}
//
//		goto UserInput;
//		break;
//	case '2':
//		show_existing_midi_devices(midi_in);
//
//		while (user_select_midi_device(midi_in) == -1) {}
//
//		goto UserInput;
//		break;
//	default:
//		printf("Good bye");
//		goto Cleanup;
//		break;
//	}
//	
//
//	// Cleanup 
//Cleanup:
//	RelinquishVJD(vjoy_device_id);
//	delete midi_in;
//
//	// Get outta heaaaa!
//	return 0;
//}