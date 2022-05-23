#pragma once

#include <glad/glad.h> // NOTE: MUST BE BEFORE GLFW

#include <GLFW/glfw3.h>

#include "include/shader.hh"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
// #include <memory>
#include <iterator>
#include <optional>
#include <thread>

namespace fs = std::filesystem;

namespace {
using V2i = std::array<int, 2>;
using V2f = std::array<float, 2>;
using V3f = std::array<float, 3>;

constexpr float gray = 0.227451f;
constexpr float TARGET_FPS = 60;
constexpr auto POINT_ACTIVATION_RADIOUS = .15;
constexpr auto POINT_ACTIVATION_RADIOUS2 =
    POINT_ACTIVATION_RADIOUS * POINT_ACTIVATION_RADIOUS;
constexpr auto gl_line_width = 1.;
constexpr auto gl_point_size = 10.0f;

struct WorldState {
  V2i win_sz = {1366, 768};
  /* V2i win_sz = {600, 600}; */
  bool mouse_down = false;
  int selected_point_idx = 0;

  std::array<V2f, 4> point_pos = {
      -.5, -.5, //
      0,   .5,  //
      .5,  0,   //
      .5,  -.5,
  };
  std::array<V3f, 4> point_color = {
      .1, .2, .9, //
      0,  .9, 0,  //
      .9, .9, 0,  //
      .9, .1, .1,
  };
  GLuint VAO_bezier, VAO_overlay;
  GLuint VBO_pos, VBO_color;

  auto CursorUV(double x, double y) const -> V2f { // shitty glfw interface
    auto xx = static_cast<float>(x);
    auto yy = static_cast<float>(y);
    return {std::clamp(2 * (xx / win_sz[0] - .5f), -1.f, 1.f),
            std::clamp(2 * ((win_sz[1] - yy) / win_sz[1] - .5f), -1.f, 1.f)};
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

inline float length2(const V2f &a, const V2f &b) {
  auto dx = a[0] - b[0], dy = a[1] - b[1];
  return dx * dx + dy * dy;
}

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
      for (int i = 0; i < ws.point_pos.size(); i++) {
        if (auto dist = length2(uv, ws.point_pos[i]); min_dist > dist) {
          min_dist = dist;
          idx = i;
        }
      }

      if (idx >= 0 && min_dist < POINT_ACTIVATION_RADIOUS2) {
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
  glBindBuffer(GL_ARRAY_BUFFER, ws.VBO_pos);
  glBufferSubData(GL_ARRAY_BUFFER, ws.selected_point_idx * sizeof(V2f),
                  sizeof(V2f),
                  std::next(ws.point_pos.begin(), ws.selected_point_idx));
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow *, int w, int h) {
  glViewport(0, 0, w, h);
  ws.win_sz[1] = h;
  ws.win_sz[0] = w;
}

} // namespace input
} // namespace
