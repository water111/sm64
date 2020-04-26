#include "Window.h"

#include "pc/imgui/imgui.h"
#include "pc/imgui/imgui_impl_glfw.h"
#include "pc/imgui/imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>

constexpr bool ENABLE_VSYNC = true;

static void glfw_error_callback(int error, const char* description) {
  printf("GLFW error: %s (%d)\n", description, error);
}

Window::Window() {
  // load GLFW
  glfwSetErrorCallback(glfw_error_callback);
  if(!glfwInit()) {
    printf("[Window] Failed to initialize GLFW\n");
  }

  // setup window
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  _window = glfwCreateWindow(1280, 720, "Super Mario (Renderer V2)", nullptr, nullptr);
  if(!_window) {
    printf("[Window] Failed to create window\n");
  }

  glfwMakeContextCurrent(_window);
  glfwSwapInterval(ENABLE_VSYNC);

  // load GLEW
  if(glewInit() != GLEW_OK) {
    printf("[Window] Failed to init GLEW\n");
  }

  // setup IMGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplGlfw_InitForOpenGL(_window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
}




void Window::end_frame() {

  // run UI code
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  generate_ui();
  // render UI
  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(_window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // swap buffers!
  glfwSwapBuffers(_window);
  if(double_swap) {
    glfwSwapBuffers(_window);
  }

  last_frame_sec = frame_timer.getSeconds();
  ft_avg = (ft_avg * 0.95) + (last_frame_sec * 0.05);
  frame_timer.start();

  glfwPollEvents();
  _frame++;
}

void Window::generate_ui() {
  ImGui::Begin("Setting");
  ImGui::TextColored(ImVec4(0.9, 0.6, 0.9, 1.0), "Single Threaded Modes");
  ImGui::Separator();
  if(ImGui::Button("No-Sync")) {
    glfwSwapInterval(0);
    double_swap = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("V-Sync 60")) {
    glfwSwapInterval(1);
    double_swap = false;
  }
  ImGui::SameLine();
  if(ImGui::Button("V-Sync 30")) {
    glfwSwapInterval(1);
    double_swap = true;
  }

  ImGui::TextColored(ImVec4(0.9, 0.6, 0.9, 1.0), "Statistics");
  ImGui::Separator();
  ImGui::Text("Graphics FPS: %d (%d avg)", (int)(1.f / last_frame_sec), (int)(1.f / ft_avg));
  ImGui::Text("Renderer time: %.3f", gfx);
  ImGui::Text("DL Process time: %.3f\n", dl_copy);
  ImGui::Text("Game seconds: %.3f\n", game / 30.f);
  ImGui::Text("tris: %5d verts %5d\n", render_stats.n_tris, render_stats.n_verts);
  ImGui::Text("Draws n64: %4d ogl: %4d\n", render_stats.n_n64_drawcalls, render_stats.n_opengl_drawcalls);
  ImGui::Text("Display list entries: %d/%d\n", displaylist_stats.n_entries, displaylist_stats.total_entries);
  ImGui::Text("Display list resources: %d/%d\n", displaylist_stats.resources, displaylist_stats.total_resources);
  ImGui::End();
}