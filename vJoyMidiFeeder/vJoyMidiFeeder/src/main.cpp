#include "AppHeaders.h"

const int MIDI_MSG_NOTE_OFF = 128;
const int MIDI_MSG_NOTE_ON = 144;
const int MIDI_MSG_CC = 176;
const int MIDI_MSG_PC = 192;

int glfw_current_error_code = 0;
std::string glfw_current_error_desc = "";
int glfw_window_width = 800;
int glfw_window_height = 600;

std::vector<std::string> available_midi_ports;
int available_midi_port_count = 0;
int available_midi_port_id = 0;

void refresh_available_midi_ports(RtMidiIn* midi_in) {
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


int midi_monitor_status = 0;
int midi_monitor_channel = 0;
int midi_monitor_byte2 = 0;
int midi_monitor_byte3 = 0;
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



int main() {

	RtMidiIn* midi_in = new RtMidiIn();
	refresh_available_midi_ports(midi_in);

	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) { return -1; }

	const char* glsl_version = "#version 440";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "vJoy Midi Feeder v0.0.0", nullptr, nullptr);
	if (!window) { return -1; }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // enable vsync. might not need this.

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return -1; }

	glViewport(0, 0, glfw_window_width, glfw_window_height);

	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

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

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.6f, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		glfw_process_input(window);
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ImGui drawing starts here
		//glfwGetWindowPos(window, &glfw_window_x, &glfw_window_y);
		//ImGui::SetNextWindowPos(ImVec2(glfw_window_x, glfw_window_y));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		glfwGetWindowSize(window, &glfw_window_width, &glfw_window_height);
		ImGui::SetNextWindowSize(ImVec2(glfw_window_width, glfw_window_height));
		ImGui::Begin("vJoy Midi Feeder", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		static const char* preview_value = available_midi_ports[available_midi_port_id].c_str();

		if (ImGui::Button("Refresh")) {
			refresh_available_midi_ports(midi_in);
			preview_value = available_midi_ports[0].c_str();
		}

		ImGui::SameLine();

		ImGui::PushItemWidth(300.0f);
		if (ImGui::BeginCombo("##combo_midiports", preview_value)) {

			for (int i = 0; i < available_midi_port_count; ++i) {
				bool is_selected = available_midi_port_id == i;
				if (ImGui::Selectable(available_midi_ports[i].c_str(), is_selected)) {
					available_midi_port_id = i;
					preview_value = available_midi_ports[i].c_str();
				}
			}

			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();

		ImGui::SameLine();
		static bool midi_port_open = midi_in->isPortOpen();
		//const char* label = midi_port_open ? "Turn MIDI Port Off" : "Turn MIDI Port On";
		const ImVec4 color = ImVec4(midi_port_open ? 0.0f : 1.0f, midi_port_open ? 1.0f : 0.0f, 0.0f, 1.0f);
		const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
		if (ImGui::ColorButton("##colorbutton_toggleport", color, flags)) {
			if (midi_port_open) {
				midi_in->cancelCallback();
				midi_in->closePort();
			}
			else {
				midi_in->openPort(available_midi_port_id);
				midi_in->setCallback(&midi_monitor_callback);
			}
			midi_port_open = midi_in->isPortOpen();
		}

		if (midi_port_open) {
			if (midi_monitor_status >= MIDI_MSG_NOTE_OFF && midi_monitor_status < MIDI_MSG_NOTE_OFF + 16) {
				ImGui::Text("Note Off\nChannel: %d\nNote: %d\nVelocity: %d", midi_monitor_channel, midi_monitor_byte2, midi_monitor_byte3);
			} else if (midi_monitor_status >= MIDI_MSG_NOTE_ON && midi_monitor_status < MIDI_MSG_NOTE_ON + 16) {
				ImGui::Text("Note On\nChannel: %d\nNote: %d\nVelocity: %d", midi_monitor_channel, midi_monitor_byte2, midi_monitor_byte3);
			} else if (midi_monitor_status >= MIDI_MSG_CC && midi_monitor_status < MIDI_MSG_CC + 16) {
				ImGui::Text("Control Change\nChannel: %d\nCC: %d\nValue: %d", midi_monitor_channel, midi_monitor_byte2, midi_monitor_byte3);
			} else if (midi_monitor_status >= MIDI_MSG_PC && midi_monitor_status < MIDI_MSG_PC + 16) {
				ImGui::Text("Program Change\nChannel: %d\nPC: %d", midi_monitor_channel, midi_monitor_byte2);
			}
		}


		/*if (ImGui::Button("Test Error")) {
			imgui_show_error(420, "I'm freakin' out man!");
		}*/

		// error modal popup
		if (ImGui::BeginPopupModal("ERROR", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("GLFW Error %d: %s\n", glfw_current_error_code, glfw_current_error_desc.c_str());
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		// ImGui drawing stops here
		ImGui::Render();

		// OpenGL drawing starts here
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Handle viewport windows
		//GLFWwindow* backup_current_context = glfwGetCurrentContext();
		//ImGui::UpdatePlatformWindows();
		//ImGui::RenderPlatformWindowsDefault();
		//glfwMakeContextCurrent(backup_current_context);

		glfwSwapBuffers(window);
	}

	if (midi_in->isPortOpen()) {
		midi_in->closePort();
	}
	delete midi_in;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}