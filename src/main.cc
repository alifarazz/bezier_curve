#include "main.hh"

int main(int argc, char *argv[]) {
  using namespace std::string_literals;

  auto maybe_shader_dir = resrc::path::find_shader_folder(argc, argv);
  if (!maybe_shader_dir.has_value()) {
    std::cout << "Please specify shader directory, for example:\n\t"
              << "$ ./bezier \"../src/shader/\"\n";
    exit(-1);
  }
  const auto shader_dir_path = maybe_shader_dir.value();

  // std::setlocale(LC_ALL, "POSIX");

  // Init glfw
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Get window from glfw + init GLAD
  auto window = glfwCreateWindow(ws.win_sz[0], ws.win_sz[1],
                                 "Bezier Curve with OpenGL Tessellation",
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

  // Handle inputs
  glfwSetMouseButtonCallback(window, input::mouse_button_callback);
  glfwSetCursorPosCallback(window, input::cursor_pos_callback);
  glfwSetKeyCallback(window, input::key_callback);
  glfwSetFramebufferSizeCallback(window, input::framebuffer_size_callback);

  Shader shader_bezier, shader_overlay;
  shader_bezier //
      .attach(shader_dir_path / "bezier.vert", GL_VERTEX_SHADER)
      .attach(shader_dir_path / "bezier.tcs.glsl", GL_TESS_CONTROL_SHADER)
      .attach(shader_dir_path / "bezier.tes.glsl", GL_TESS_EVALUATION_SHADER)
      .attach(shader_dir_path / "bezier.frag", GL_FRAGMENT_SHADER)
      .link();
  shader_overlay //
      .attach(shader_dir_path / "overlay.vert", GL_VERTEX_SHADER)
      .attach(shader_dir_path / "overlay.frag", GL_FRAGMENT_SHADER)
      .link();

  // bezier shader
  glPatchParameteri(GL_PATCH_VERTICES, 4);
  glUseProgram(shader_bezier.id());
  glGenVertexArrays(1, &ws.VAO_bezier);
  glBindVertexArray(ws.VAO_bezier);

  // Vertex positions
  glGenBuffers(1, &ws.VBO_pos);

  glBindBuffer(GL_ARRAY_BUFFER, ws.VBO_pos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(V2f) * ws.point_pos.size(),
               ws.point_pos.data(), GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // overlay shader
  glUseProgram(shader_overlay.id());
  glGenVertexArrays(1, &ws.VAO_overlay);
  glBindVertexArray(ws.VAO_overlay);
  glBindBuffer(GL_ARRAY_BUFFER, ws.VBO_pos);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glGenBuffers(1, &ws.VBO_color);
  glBindBuffer(GL_ARRAY_BUFFER, ws.VBO_color);
  glBufferData(GL_ARRAY_BUFFER, sizeof(V3f) * ws.point_color.size(),
               ws.point_color.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glLineWidth(gl_line_width);
  glPointSize(gl_point_size);
  glClearColor(gray, gray, gray, 1.0f);

  std::cout << "Before render-loop: GLerror = " << glGetError() << std::endl;

  auto last_frame = std::chrono::high_resolution_clock::now();
  auto current_frame = std::chrono::high_resolution_clock::now();
  auto delta_time = current_frame - last_frame;
  constexpr float OPTIMAL_TIME = 1e9 / TARGET_FPS;

  do { /* Main render loop */
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shader_bezier.id());
    glBindVertexArray(ws.VAO_bezier);
    glDrawArrays(GL_PATCHES, 0, 4);

    glUseProgram(shader_overlay.id());
    glBindVertexArray(ws.VAO_overlay);
    glDrawArrays(GL_POINTS, 0, 4);

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
  glDeleteVertexArrays(1, &ws.VAO_overlay);
  glDeleteBuffers(1, &ws.VBO_pos);
  glDeleteBuffers(1, &ws.VBO_color);
  shader_bezier.destroy();
  glfwDestroyWindow(window);
  glfwTerminate();

  std::cout << "After cleanup: GLerror = " << glGetError() << std::endl;
}
