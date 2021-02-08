#include "AppHeaders.h"

static int glfw_current_error_code = 0;
static std::string glfw_current_error_desc = "";
static int glfw_window_width = 800;
static int glfw_window_height = 600;
static int glfw_window_x = 0;
static int glfw_window_y = 0;

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	glfw_current_error_code = error;
	glfw_current_error_desc = description;
	ImGui::OpenPopup("ERROR");
	
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



int main() {

	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) return -1;

	const char* glsl_version = "#version 440";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "vJoy Midi Feeder v0.0.0", nullptr, nullptr);
	if (!window) {
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // enable vsync. might not need this.

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		return -1;
	}

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
		glfwGetWindowPos(window, &glfw_window_x, &glfw_window_y);
		glfwGetWindowSize(window, &glfw_window_width, &glfw_window_height);
		//ImGui::SetNextWindowPos(ImVec2(glfw_window_x, glfw_window_y));
		ImGui::SetNextWindowSize(ImVec2(glfw_window_width, glfw_window_height));
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("vJoy Midi Feeder", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		if (ImGui::Button("Test Error")) {
			glfw_current_error_code = 420;
			glfw_current_error_desc = "I'm freakin' out man!";
			ImGui::OpenPopup("ERROR");
		}
		

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

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}