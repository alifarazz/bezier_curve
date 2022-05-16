#pragma once

#include <glad/glad.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>
namespace fs = std::filesystem;

class Shader {
private:
  static auto ReadFile(const fs::path &path, std::string &content) -> void;
  static auto CompileShader(unsigned int shader) -> bool;

  GLuint program_;
  std::vector<GLuint> createdShaders;

public:
  Shader &operator=(Shader &&other) = default;

  // No copy constructor
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

  Shader();

  auto destroy() -> void;

  auto attach(const fs::path &shaderPath, GLenum shaderType) -> Shader &;
  auto link() -> Shader &;
  auto id() const -> GLuint;
  auto getUniform(const std::string &name) const -> GLint;
};

