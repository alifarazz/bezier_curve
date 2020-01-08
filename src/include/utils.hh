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
    using stbi_uc_UniquePtr = std::unique_ptr<stbi_uc*, DestorySTBIImageStruct>;
    auto loadImageFromFile(const fs::path& path)
        -> std::tuple<stbi_uc*,
            int,
            int,
            int>
    {
        int x, y, comp;
        stbi_uc* data = stbi_load(path.c_str(), &x, &y, &comp, 0);
        // std::unique_ptr<stbi_uc *, decltype(free_image)> &&
        // std::unique_ptr<stbi_uc *, decltype(free_image)>(u, free_image).get()
        return std::make_tuple(data, x, y, comp);
    }
}
}