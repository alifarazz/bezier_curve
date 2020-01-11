#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hh"
#include "utils.hh"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>
namespace fs = std::filesystem;

struct WorldState {
  using v2 = glm::vec2;

  v2 cursor_offset; // cursor offset vector
  int selected_point = 0;
  std::array<glm::vec2, 3> point_pos = {
      v2{-.5, -.5},
      v2{0, .5},
      v2{.5, -.5},
  };
  std::array<glm::vec2, 3> point_uv = {
      v2{0, 0},
      v2{.5, 0},
      v2{1, 1},
  };
  GLuint VAO_bezier;
  GLuint VBO_uv;
  GLuint VBO_pos;

  auto updateXY(Utils::GLFWwindowUniquePtr &window, int idx) -> void {
    double x, y;
    glfwGetCursorPos(window.get(), &x, &y);
    this->cursor_offset.x = point_pos[idx].x - x;
    this->cursor_offset.y = point_pos[idx].y - y;
  }
} world_state;

constexpr int window_height{1366}, window_width{768};
constexpr double aspect_ratio =
    static_cast<double>(window_width) / window_height;

int main() {
  using namespace std::string_literals;

  // std::setlocale(LC_ALL, "POSIX");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  Utils::GLFWwindowUniquePtr window{
      glfwCreateWindow(window_height, window_width,
                       "Shadow Mapping Example OpenGL", nullptr, nullptr)};
  glfwMakeContextCurrent(window.get());
  if (!window.get())
    throw std::runtime_error{"glfw window create failed"};
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    throw std::runtime_error{"GLAD init failed"};

  // Adjust viewport upon window resize
  glfwSetFramebufferSizeCallback(
      window.get(), [](auto, int w, int h) { glViewport(0, 0, w, h); });
  glfwSetScrollCallback(window.get(), [](auto, auto, auto) {});

  // Grab cursor
  glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  const auto base_path{fs::current_path() / "../../"};
  Shader shader_bezier;
  shader_bezier.attach(base_path / "shader/bezier.vert", GL_VERTEX_SHADER)
      .attach(base_path / "shader/bezier.frag", GL_FRAGMENT_SHADER)
      .link();

  // Buffers
  glUseProgram(shader_bezier.id());
  glGenVertexArrays(1, &world_state.VAO_bezier);
  glBindVertexArray(world_state.VAO_bezier);

  glGenBuffers(1, &world_state.VBO_pos);
  glGenBuffers(1, &world_state.VBO_uv);

  glBindBuffer(GL_ARRAY_BUFFER, world_state.VBO_pos);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(glm::vec2) * world_state.point_pos.size(),
               world_state.point_pos.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, world_state.VBO_uv);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * world_state.point_uv.size(),
               world_state.point_uv.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glfwSetCursorPosCallback(window.get(), [](GLFWwindow *, double x, double y) {
    // God this code is bad...
    const int xint = static_cast<int>(x);
    const int yint = static_cast<int>(y);
    x = static_cast<double>(xint) / window_width;
    y = static_cast<double>(yint) / window_height;
    y /= aspect_ratio;
    world_state.point_pos[world_state.selected_point].x = x;
    world_state.point_pos[world_state.selected_point].y = -y;
        // glm::vec2{static_cast<float>(x)  + world_state.cursor_offset.x,
        //           static_cast<float>(-y) + world_state.cursor_offset.y};

    glBindBuffer(GL_ARRAY_BUFFER, world_state.VBO_pos);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(glm::vec2) * world_state.point_pos.size(),
                 world_state.point_pos.data(), GL_DYNAMIC_DRAW);
  });

  glPointSize(5.0f);
  glClearColor(0.227451f, 0.227451f, 0.227451f, 1.0f);
  auto last_frame{std::chrono::high_resolution_clock::now()};
  auto current_frame{std::chrono::high_resolution_clock::now()};
  auto delta_time{current_frame - last_frame};
  constexpr int TARGET_FPS = 60;
  constexpr float OPTIMAL_TIME{1e9 / TARGET_FPS};
  do {
    if (glfwGetKey(window.get(), GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window.get(), true);
    if (glfwGetKey(window.get(), GLFW_KEY_1) == GLFW_PRESS) {
      world_state.updateXY(window, 0);
      world_state.selected_point = 0;
    }
    if (glfwGetKey(window.get(), GLFW_KEY_2) == GLFW_PRESS) {
      world_state.updateXY(window, 1);
      world_state.selected_point = 1;
    }
    if (glfwGetKey(window.get(), GLFW_KEY_3) == GLFW_PRESS) {
      world_state.updateXY(window, 2);
      world_state.selected_point = 2;
    }
    if (glfwGetKey(window.get(), GLFW_KEY_4) == GLFW_PRESS)
      ;

    /* BEGIN RENDER */
    // glBindVertexArray(world_state.VAO_bezier);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_bezier.id());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    // glDrawArrays(GL_POINTS, 0, 3);  // looks like ...!

    glfwSwapBuffers(window.get());
    glfwPollEvents();

    current_frame = std::chrono::high_resolution_clock::now();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    std::this_thread::sleep_for(std::chrono::duration<float, std::nano>(
        std::max(0.0, OPTIMAL_TIME - delta_time.count() * 1e-9)));
  } while (!glfwWindowShouldClose(window.get()));

  /* CLEAN-UP */
  glDeleteVertexArrays(1, &world_state.VAO_bezier);
  glDeleteBuffers(1, &world_state.VBO_pos);
  glDeleteBuffers(1, &world_state.VBO_uv);
  glfwTerminate();
  return 0;
}
