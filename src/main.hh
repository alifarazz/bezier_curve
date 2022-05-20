#pragma once

#include <glad/glad.h> // NOTE: MUST BE BEFORE GLFW

#include <GLFW/glfw3.h>

#include <Imath/ImathVec.h>

#include "include/shader.hh"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
// #include <memory>
#include <optional>
#include <thread>

namespace fs = std::filesystem;

namespace {
constexpr float TARGET_FPS = 60;
constexpr auto POINT_ACTIVATION_RADIOUS = .15;
constexpr auto POINT_ACTIVATION_RADIOUS2 =
    POINT_ACTIVATION_RADIOUS * POINT_ACTIVATION_RADIOUS;
constexpr auto gl_line_width = 1.;
constexpr auto gl_point_size = 10.0f;
constexpr auto gray = Imath::V3f(0.227451f);

struct WorldState {
  Imath::V2i win_sz = {1366, 767};
  /* Imath::V2i win_sz = {600, 600}; */
  bool mouse_down = false;
  int selected_point_idx = 0;

  using v2 = Imath::V2f;
  using v3 = Imath::V3f;
  std::array<v2, 4> point_pos = {
      v2{-.5, -.5},
      v2{0, .5},
      v2{.5, 0},
      v2{.5, -.5},
  };
  std::array<v3, 4> point_color = {
      v3{.1, .2, .9},
      v3{0, .9, 0},
      v3{.9, .9, 0},
      v3{.9, .1, .1},
  };
  GLuint VAO_bezier, VAO_overlay;
  GLuint VBO_pos, VBO_color;

  auto CursorUV(double x, double y) const
      -> Imath::V2f { // shitty glfw interface
    return Imath::V2f(
        std::clamp(2 * (x / win_sz.x - .5), -1., 1.),
        std::clamp(2 * ((win_sz.y - y) / win_sz.y - .5), -1., 1.));
  }
} ws;

namespace resrc::path {
std::optional<fs::path> find_shader_folder(int argc, char *argv[]) {
  if (auto path = (argc < 2) ? fs::current_path() / "../src/shader/"
                             : fs::path{argv[1]};
      fs::exists(path))
    return path;
  return {};
}
} // namespace resrc::path

namespace input {
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    if (action == GLFW_PRESS) {
      double x, y;
      glfwGetCursorPos(window, &x, &y);

      auto uv = ws.CursorUV(x, y);

      float min_dist =
          std::numeric_limits<float>::max(); // std::min_element and std::min
      int idx = -1;
      for (int i = 0; i < ws.point_pos.size(); i++)
        if (auto dist = (uv - ws.point_pos[i]).length2(); min_dist > dist) {
          min_dist = dist;
          idx = i;
        }

      if (idx >= 0 && min_dist < POINT_ACTIVATION_RADIOUS) {
        ws.mouse_down = true;
        // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ws.selected_point_idx = idx;
      }
    } else if (action == GLFW_RELEASE && ws.mouse_down == true) {
      ws.mouse_down = false;
      // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
  }
}

void cursor_pos_callback(GLFWwindow *, double x, double y) {
  if (!ws.mouse_down)
    return;

  auto uv = ws.CursorUV(x, y);
  ws.point_pos[ws.selected_point_idx] = uv;
  glBindBuffer(GL_ARRAY_BUFFER,
               ws.VBO_pos); // FIXME: only copy the value which is changed
  glBufferData(GL_ARRAY_BUFFER, sizeof(Imath::V2f) * ws.point_pos.size(),
               ws.point_pos.data(), GL_DYNAMIC_DRAW);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *, int w, int h) {
  glViewport(0, 0, w, h);
  ws.win_sz.y = h;
  ws.win_sz.x = w;
}

} // namespace input
} // namespace
