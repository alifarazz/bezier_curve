cmake_minimum_required(VERSION 3.16)

project(
  bezier
  VERSION 1.0.0
  LANGUAGES CXX C)

set(PROJECT_IS_MAIN (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME))

include(cmake/get_cpm.cmake)

CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.3.7
        OPTIONS
          "GLFW_BUILD_TESTS OFF"
          "GLFW_BUILD_EXAMPLES OFF"
          "GLFW_BULID_DOCS OFF"
          "GLFW_BUILD_INSTALL OFF"
)
CPMAddPackage("https://github.com/g-truc/glm.git#0.9.9.8")
find_package(OpenGL REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cc
    src/include/shader.hh
    external/glad/src/glad.c)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC external/glad/include/)
target_link_libraries(${PROJECT_NAME} glfw glm::glm OpenGL::GL ${CMAKE_DL_LIBS})
