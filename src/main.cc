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
constexpr float TARGET_FPS = 60;
constexpr auto POINT_ACTIVATION_RADIOUS = .15;

struct WorldState {
  glm::i32vec2 win_sz = {1366, 767};
  /* glm::i32vec2 win_sz = {600, 600}; */
  bool mouse_down = false;
  int selected_point_idx = 0;

  using v2 = glm::vec2;
  std::array<glm::vec2, 3> point_pos = {
      v2{-.5, -.5},
      v2{0, .5},
      v2{.5, -.5},
  };
  GLuint VAO_bezier;
  GLuint VBO_pos;

  auto CursorUV(double x, double y) const
      -> glm::vec2 { // shitty glfw interface
    return {std::clamp(2 * (x / win_sz.x - .5), -1., 1.),
            std::clamp(2 * ((win_sz.y - y) / win_sz.y - .5), -1., 1.)};
  }
} ws;
} // namespace

int main(int argc, char *argv[]) {
  using namespace std::string_literals;

  if (argc != 2) {
    std::cout << "Please specify shader directory, for example:\n\t"
              << "$ ./bezier \"../src/shader/\"\n";
    exit(-1);
  }
  const auto shader_dir_path = std::filesystem::path{argv[1]};

  // std::setlocale(LC_ALL, "POSIX");

  // Init glfw and set OpenGL version to 3.3 core
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  /* glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); */

  // Get window from glfw + init GLAD
  auto window = glfwCreateWindow(ws.win_sz.x, ws.win_sz.y, "Bezier OpenGL",
                                 nullptr, nullptr);
  if (window == nullptr) {
    std::cerr << "glfw window create failed" << std::endl;
    exit(-2);
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
    std::cerr << "GLAD init failed" << std::endl;
    exit(-3);
  }

  // Set callback to adjust viewport upon window resize
  glfwSetFramebufferSizeCallback(window, [](auto, int w, int h) {
    glViewport(0, 0, w, h);
    ws.win_sz.y = h;
    ws.win_sz.x = w;
  });

  Shader shader_bezier;
  shader_bezier
    .attach(shader_dir_path / "bezier.vert", GL_VERTEX_SHADER)
    .attach(shader_dir_path / "bezier.frag", GL_FRAGMENT_SHADER)
    .link();

  // Buffers
  glUseProgram(shader_bezier.id());
  glGenVertexArrays(1, &ws.VAO_bezier);
  glBindVertexArray(ws.VAO_bezier);

  glGenBuffers(1, &ws.VBO_pos);

  // Vertex positions
  glBindBuffer(GL_ARRAY_BUFFER, ws.VBO_pos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * ws.point_pos.size(),
               ws.point_pos.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Handle mouse input
  // Handle mouse buttons
  glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button,
                                        int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
      if (action == GLFW_PRESS) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);

        auto uv = ws.CursorUV(x, y);
        auto pred = [uv](const auto pos) {
          return glm::distance(uv, pos) < POINT_ACTIVATION_RADIOUS;
        };
        auto it = std::find_if(ws.point_pos.begin(), ws.point_pos.end(), pred);

        if (it != ws.point_pos.end()) {
          ws.mouse_down = true;
          glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
          ws.selected_point_idx = std::distance(ws.point_pos.begin(), it);
        }
      } else if (action == GLFW_RELEASE && ws.mouse_down == true) {
        ws.mouse_down = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
    }
  });
  // Handle mouse cursor position
  glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y) {
    if (!ws.mouse_down)
      return;

    auto uv = ws.CursorUV(x, y);
    ws.point_pos[ws.selected_point_idx] = uv;
    glBindBuffer(GL_ARRAY_BUFFER, ws.VBO_pos); // FIXME: only copy the value which is changed
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * ws.point_pos.size(),
                 ws.point_pos.data(), GL_DYNAMIC_DRAW);
  });
  // Hanle keyboard input
  glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode,
                                int action, int mods) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
  });

  glPointSize(10.0f);
  glClearColor(0.227451f, 0.227451f, 0.227451f, 1.0f);

  auto last_frame = std::chrono::high_resolution_clock::now();
  auto current_frame = std::chrono::high_resolution_clock::now();
  auto delta_time = current_frame - last_frame;
  constexpr float OPTIMAL_TIME = 1e9 / TARGET_FPS;
  do { /* Main render loop */
    // glBindVertexArray(ws.VAO_bezier); // no need, algready bound
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
  glDeleteVertexArrays(1, &ws.VAO_bezier);
  glDeleteBuffers(1, &ws.VBO_pos);
  shader_bezier.destroy();
  glfwDestroyWindow(window);
  glfwTerminate();
}
