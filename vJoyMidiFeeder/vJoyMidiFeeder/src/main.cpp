#include "AppHeaders.h"

const char* glsl_version = "#version 440";
GLFWwindow* glfw_window = nullptr;
ImVec4 glfw_window_bg_color = ImVec4(0.4f, 0.5f, 0.6f, 1.0f);

const int MIDI_MSG_NOTE_OFF = 128;
const int MIDI_MSG_NOTE_ON = 144;
const int MIDI_MSG_CC = 176;
const int MIDI_MSG_PC = 192;
RtMidiIn* midi_in = nullptr;
int midi_monitor_status = 0;
int midi_monitor_channel = 0;
int midi_monitor_byte2 = 0;
int midi_monitor_byte3 = 0;

int glfw_current_error_code = 0;
std::string glfw_current_error_desc = "";
int glfw_window_width = 800;
int glfw_window_height = 600;

std::vector<std::string> available_midi_ports;
int available_midi_port_count = 0;
int available_midi_port_id = 0;

void refresh_available_midi_ports() {

	available_midi_ports.clear();
	available_midi_port_count = midi_in->getPortCount();
	if (available_midi_port_count == 0) {
		available_midi_ports.push_back("No available MIDI ports.");
	}
	else {
		for (int i = 0; i < available_midi_port_count; ++i) {
			std::string port_name = midi_in->getPortName(i);
			available_midi_ports.push_back(port_name);
		}
	}
}

void midi_monitor_callback(double deltatime, std::vector< unsigned char > *message, void *userData)
{
	midi_monitor_status = (int)message->at(0);

	if (midi_monitor_status >= MIDI_MSG_NOTE_OFF && midi_monitor_status < MIDI_MSG_NOTE_OFF + 16) {
		midi_monitor_channel = midi_monitor_status - MIDI_MSG_NOTE_OFF + 1;
		midi_monitor_byte2 = (int)message->at(1); // key number
		midi_monitor_byte3 = (int)message->at(2); // velocity
		return;
	}

	if (midi_monitor_status >= MIDI_MSG_NOTE_ON && midi_monitor_status < MIDI_MSG_NOTE_ON + 16) {
		midi_monitor_channel = midi_monitor_status - MIDI_MSG_NOTE_ON + 1;
		midi_monitor_byte2 = (int)message->at(1); // key number
		midi_monitor_byte3 = (int)message->at(2); // velocity
		return;
	}

	if (midi_monitor_status >= MIDI_MSG_CC && midi_monitor_status < MIDI_MSG_CC + 16) {
		midi_monitor_channel = midi_monitor_status - MIDI_MSG_CC + 1;
		midi_monitor_byte2 = (int)message->at(1); // cc id
		midi_monitor_byte3 = (int)message->at(2); // cc value
		return;
	}

	if (midi_monitor_status >= MIDI_MSG_PC && midi_monitor_status < MIDI_MSG_PC + 16) {
		midi_monitor_channel = midi_monitor_status - MIDI_MSG_PC + 1;
		midi_monitor_byte2 = (int)message->at(1); // program number
		return;
	}
}

void imgui_show_error(int code, const char* desc) {
	glfw_current_error_code = code;
	glfw_current_error_desc = desc;
	ImGui::OpenPopup("ERROR");
}

void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	imgui_show_error(error, description);	
}

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	glfw_window_width = width;
	glfw_window_height = height;
}

void glfw_process_input(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

void imgui_midi_port_manager() {
	static const char* combo_preview_available_midi_ports = available_midi_ports[available_midi_port_id].c_str();

	if (ImGui::Button("Refresh") && midi_in->isPortOpen() == false) {
		refresh_available_midi_ports();
		combo_preview_available_midi_ports = available_midi_ports[0].c_str();
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(300.0f);
	if (ImGui::BeginCombo("##combo_midiports", combo_preview_available_midi_ports)) {

		for (int i = 0; i < available_midi_port_count; ++i) {
			bool is_selected = available_midi_port_id == i;
			if (ImGui::Selectable(available_midi_ports[i].c_str(), is_selected)) {
				if (midi_in->isPortOpen()) continue;
				available_midi_port_id = i;
				combo_preview_available_midi_ports = available_midi_ports[i].c_str();
			}
		}

		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	ImGui::SameLine();
	
	if (available_midi_port_count > 0) {
		bool midi_port_open = midi_in->isPortOpen();
		const ImVec4 color = ImVec4(midi_port_open ? 0.0f : 1.0f, midi_port_open ? 1.0f : 0.0f, 0.0f, 1.0f);
		const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
		if (ImGui::ColorButton("##colorbutton_toggleport", color, flags)) {
			if (midi_in->isPortOpen()) {
				midi_in->cancelCallback();
				midi_in->closePort();
			}
			else {
				midi_in->openPort(available_midi_port_id);
				midi_in->setCallback(&midi_monitor_callback);
			}
			//midi_port_open = midi_in->isPortOpen();
		}
	}
}

void imgui_midi_monitor() {
	if (midi_in->isPortOpen()) {
		if (midi_monitor_status >= MIDI_MSG_NOTE_OFF && midi_monitor_status < MIDI_MSG_NOTE_OFF + 16) {
			ImGui::Text("Note Off\nChannel: %d\nNote: %d\nVelocity: %d", midi_monitor_channel, midi_monitor_byte2, midi_monitor_byte3);
		}
		else if (midi_monitor_status >= MIDI_MSG_NOTE_ON && midi_monitor_status < MIDI_MSG_NOTE_ON + 16) {
			ImGui::Text("Note On\nChannel: %d\nNote: %d\nVelocity: %d", midi_monitor_channel, midi_monitor_byte2, midi_monitor_byte3);
		}
		else if (midi_monitor_status >= MIDI_MSG_CC && midi_monitor_status < MIDI_MSG_CC + 16) {
			ImGui::Text("Control Change\nChannel: %d\nCC: %d\nValue: %d", midi_monitor_channel, midi_monitor_byte2, midi_monitor_byte3);
		}
		else if (midi_monitor_status >= MIDI_MSG_PC && midi_monitor_status < MIDI_MSG_PC + 16) {
			ImGui::Text("Program Change\nChannel: %d\nPC: %d", midi_monitor_channel, midi_monitor_byte2);
		}
	}
}

void imgui_error_popup() {
	// error modal popup
	if (ImGui::BeginPopupModal("ERROR", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("GLFW Error %d: %s\n", glfw_current_error_code, glfw_current_error_desc.c_str());
		if (ImGui::Button("Close")) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void app_setup_midi() {
	midi_in = new RtMidiIn();
	refresh_available_midi_ports();
}

int app_setup_glfw() {
	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) { return -1; }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfw_window = glfwCreateWindow(800, 600, "vJoy Midi Feeder v0.0.0", nullptr, nullptr);
	if (!glfw_window) { return -1; }

	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1); // enable vsync. might not need this.

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return -1; }

	glfwSetFramebufferSizeCallback(glfw_window, glfw_framebuffer_size_callback);
	return 0;
}

void app_setup_imgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();


	//ImGuiStyle& style = ImGui::GetStyle();
	//style.WindowRounding = 0.0f;
	//style.Colors[ImGuiCol_WindowBg].w = 1.0f;

	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

int main() {

	app_setup_midi();
	app_setup_glfw();
	app_setup_imgui();

	while (!glfwWindowShouldClose(glfw_window)) {
		glfw_process_input(glfw_window);
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui drawing starts here
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		glfwGetWindowSize(glfw_window, &glfw_window_width, &glfw_window_height);
		ImGui::SetNextWindowSize(ImVec2(glfw_window_width, glfw_window_height));
		ImGui::Begin("vJoy Midi Feeder", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		imgui_midi_port_manager();
		imgui_midi_monitor();

		/*if (ImGui::Button("Test Error")) {
			imgui_show_error(420, "I'm freakin' out man!");
		}*/
		imgui_error_popup();
		

		ImGui::End();

		// ImGui drawing stops here
		ImGui::Render();

		// OpenGL drawing starts here
		glClearColor(glfw_window_bg_color.x, glfw_window_bg_color.y, glfw_window_bg_color.z, glfw_window_bg_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Handle viewport windows
		//GLFWwindow* backup_current_context = glfwGetCurrentContext();
		//ImGui::UpdatePlatformWindows();
		//ImGui::RenderPlatformWindowsDefault();
		//glfwMakeContextCurrent(backup_current_context);

		glfwSwapBuffers(glfw_window);
	}

	if (midi_in->isPortOpen()) {
		midi_in->closePort();
	}
	delete midi_in;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(glfw_window);

	glfwTerminate();
	return 0;
}