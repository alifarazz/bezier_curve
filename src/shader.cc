#include "include/shader.hh"

Shader::Shader() { program_ = glCreateProgram(); }

auto Shader::destroy() -> void { glDeleteProgram(program_); }

// Shader &Shader::operator=(Shader &&other) {
//   program_ = other.program_;
//   other.program_ = 0;
//   if (other.program_ != 0)
//     glDeleteProgram(other.program_);
//   return *this;
// }

auto Shader::attach(const fs::path &shaderPath, GLenum shaderType) -> Shader & {
  std::string shdr_src;
  try {
    ReadFile(shaderPath, shdr_src);
  } catch (std::ifstream::failure &e) {
    std::cerr << "Shader:: failed to open shader source files\n"
              << "\tpath: " << shaderPath << "\n\t" << e.what() << std::endl;
    shdr_src = "";
  }

  GLuint shdr{glCreateShader(shaderType)};
  const GLchar *source = shdr_src.data();
  glShaderSource(shdr, 1, &source, nullptr);
  // std::clog << "SHADER::CompileShader:: compiling shader: "
  //           << shaderPath << std::endl;
  if (CompileShader(shdr))
    std::clog << "LOG::Shader::\"Compiling Shader Successful\": " << shaderPath
              << std::endl;

  createdShaders.push_back(shdr);
  glAttachShader(program_, shdr);
  return *this;
}

auto Shader::link() -> Shader & {
  /* code below return 0 insted of name of attached shaders */
  // GLint shaderCount;
  // glGetProgramiv(program_, GL_ATTACHED_SHADERS, &shaderCount);
  // std::cout << shaderCount << std::endl;
  // std::vector<GLuint> shaders(shaderCount);
  // glGetAttachedShaders(program_, 0, nullptr, shaders.data());

  glLinkProgram(program_);

  for (std::size_t i = 0; i < createdShaders.size(); i++) {
    // glDetachShader(program_, shaders[i]);
    // glDeleteShader(shaders[i]);
    glDetachShader(program_, createdShaders[i]);
    glDeleteShader(createdShaders[i]);
  }
  createdShaders.clear();

  GLint isLinked = 0;
  glGetProgramiv(program_, GL_LINK_STATUS, &isLinked);
  if (isLinked == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &maxLength);

    // The maxLength includes the NULL character
    std::vector<GLchar> log(static_cast<std::size_t>(maxLength));
    glGetProgramInfoLog(program_, maxLength, &maxLength, log.data());
    std::cerr << "Shader:: failed to link shader program" << std::endl;
    std::copy(begin(log), end(log),
              std::ostream_iterator<GLchar>{std::cerr, ""});

    glDeleteProgram(program_); // Don't leak the program.
  } else {
    std::clog << "LOG::Shader::\"Linking Shader Program Successful\""
              << std::endl;
  }
  // Program is linked successfully.
  return *this;
}

auto Shader::id() const -> GLuint { return program_; }

auto Shader::getUniform(const std::string &name) const -> GLint {
  return glGetUniformLocation(program_, name.c_str());
}

auto Shader::CompileShader(GLuint shader) -> bool {
  glCompileShader(shader);

  GLint isCompiled = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    std::cerr << "Shader:: failed to compile shader program" << std::endl;

    // The maxLength includes the NULL character
    std::vector<GLchar> log(static_cast<std::size_t>(maxLength));
    glGetShaderInfoLog(shader, maxLength, &maxLength, log.data());
    std::copy(begin(log), end(log),
              std::ostream_iterator<GLchar>{std::cerr, ""});

    glDeleteShader(shader); // Don't leak the shader.
    return false;
  }
  return true; // Shader compilation is successful.
}

auto Shader::ReadFile(const fs::path &path, std::string &content) -> void {
  std::ifstream ifs(path);
  ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  auto r{std::string((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()))};
  content = std::move(r);
}
