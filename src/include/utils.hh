#pragma once
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <filesystem>
#include <iostream>
#include <memory>
#include <tuple>
namespace fs = std::filesystem;

namespace {
namespace Utils {
    struct DestoryGLFWWindowStruct {
        auto operator()(GLFWwindow* ptr) -> void { glfwDestroyWindow(ptr); }
    };
    struct DestorySTBIImageStruct {
        auto operator()(stbi_uc* x) -> void { stbi_image_free(x); }
    };
    using GLFWwindowUniquePtr = std::unique_ptr<GLFWwindow, DestoryGLFWWindowStruct>;
}
}