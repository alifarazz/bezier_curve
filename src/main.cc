#include <glad/glad.h> // NOTE: MUST BE BEFORE GLFW

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "include/shader.hh"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

namespace fs = std::filesystem;

namespace {
constexpr int TARGET_FPS = 60;
constexpr int window_height{1366}, window_width{768};
constexpr double aspect_ratio =
    static_cast<double>(window_width) / window_height;

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

  auto getCursorXY(GLFWwindow *window) -> v2 {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return convertCursorXY(x, y);
  }

  auto convertCursorXY(double x, double y) -> v2 {
    // God this code is bad...
    const int xint = static_cast<int>(x);
    const int yint = static_cast<int>(y);
    x = static_cast<double>(xint) / window_width;
    y = static_cast<double>(yint) / window_height;
    y /= aspect_ratio;
    return v2{x, y};
  }

  auto updateXY(GLFWwindow *window, int idx) -> void {
    auto v = getCursorXY(window);
    v = convertCursorXY(v.x, v.y);
    this->cursor_offset.x = point_pos[idx].x - v.x;
    this->cursor_offset.y = point_pos[idx].y - v.y;
  }
} world_state;
} // namespace
// TODO:
//  implement circle SDF for points

int main(int argc, char *argv[]) {
  using namespace std::string_literals;

  if (argc != 2) {
    std::cout << "Please specify shader directory, for example:\n\t"
              << "$ ./bezier \"../src/shader/\"\n";
    exit(-1);
  }
  const auto shader_dir_path = std::filesystem::path{argv[1]};

  // std::setlocale(LC_ALL, "POSIX");

  // Init glfw and set OpenGL version to 4.1 core
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  // Get window from glfw + init GLAD
  auto window = glfwCreateWindow(window_height, window_width, "Bezier OpenGL",
                                 nullptr, nullptr);
  if (window == nullptr)
    throw std::runtime_error{"glfw window create failed"};
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress)))
    throw std::runtime_error{"GLAD init failed"};

  // Set callback to adjust viewport upon window resize
  glfwSetFramebufferSizeCallback(
      window, [](auto, int w, int h) { glViewport(0, 0, w, h); });
  glfwSetScrollCallback(window,
                        [](auto, auto, auto) {}); // REVIEW: is this necessary?

  // Grab cursor
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  Shader shader_bezier;
  shader_bezier.attach(shader_dir_path / "bezier.vert", GL_VERTEX_SHADER)
      .attach(shader_dir_path / "bezier.frag", GL_FRAGMENT_SHADER)
      .link();

  // Buffers
  glUseProgram(shader_bezier.id());
  glGenVertexArrays(1, &world_state.VAO_bezier);
  glBindVertexArray(world_state.VAO_bezier);

  glGenBuffers(1, &world_state.VBO_pos);
  glGenBuffers(1, &world_state.VBO_uv);

  // Vertex positions
  glBindBuffer(GL_ARRAY_BUFFER, world_state.VBO_pos);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(glm::vec2) * world_state.point_pos.size(),
               world_state.point_pos.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Vertex UV coords
  glBindBuffer(GL_ARRAY_BUFFER, world_state.VBO_uv);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * world_state.point_uv.size(),
               world_state.point_uv.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Handle mouse input
  glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y) {
    auto v = world_state.convertCursorXY(x, y);
    world_state.point_pos[world_state.selected_point] = glm::vec2{
        v.x + world_state.cursor_offset.x, -v.y + world_state.cursor_offset.y};
    glBindBuffer(GL_ARRAY_BUFFER, world_state.VBO_pos);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(glm::vec2) * world_state.point_pos.size(),
                 world_state.point_pos.data(), GL_DYNAMIC_DRAW);
  });

  glPointSize(10.0f);
  glClearColor(0.227451f, 0.227451f, 0.227451f, 1.0f);

  auto last_frame{std::chrono::high_resolution_clock::now()};
  auto current_frame{std::chrono::high_resolution_clock::now()};
  auto delta_time{current_frame - last_frame};
  constexpr float OPTIMAL_TIME{1e9 / TARGET_FPS};
  do {
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
      world_state.updateXY(window, 0);
      world_state.selected_point = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
      world_state.updateXY(window, 1);
      world_state.selected_point = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
      world_state.updateXY(window, 2);
      world_state.selected_point = 2;
    }
    /* if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS); //TODO */

    /* BEGIN RENDER */
    // glBindVertexArray(world_state.VAO_bezier); // no need, algready bound
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_bezier.id());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
    glDrawArrays(GL_POINTS, 0, 3); // DEBUG, make sure you set glPointSize

    glfwSwapBuffers(window);
    glfwPollEvents();

    current_frame = std::chrono::high_resolution_clock::now();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;

    std::this_thread::sleep_for(std::chrono::duration<float, std::nano>(
        std::max(0.0, OPTIMAL_TIME - delta_time.count() * 1e-9)));
  } while (!glfwWindowShouldClose(window));

  /* CLEAN-UP */
  glDeleteVertexArrays(1, &world_state.VAO_bezier);
  glDeleteBuffers(1, &world_state.VBO_pos);
  glDeleteBuffers(1, &world_state.VBO_uv);
  shader_bezier.destroy();
  glfwDestroyWindow(window);
  glfwTerminate();
}
