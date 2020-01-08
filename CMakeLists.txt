cmake_minimum_required(VERSION 3.0)
project(bezier)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(vendor/glfw)

# option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
# option(ASSIMP_BUILD_SAMPLES OFF)
# option(ASSIMP_BUILD_TESTS OFF)
# add_subdirectory(vendor/assimp)

# option(BUILD_BULLET2_DEMOS OFF)
# option(BUILD_CPU_DEMOS OFF)
# option(BUILD_EXTRAS OFF)
# option(BUILD_OPENGL3_DEMOS OFF)
# option(BUILD_UNIT_TESTS OFF)
# add_subdirectory(vendor/bullet)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4") # idk
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17 -pipe  -ggdb")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(src/include/
                    # vendor/assimp/include/
                    vendor/stb/
                    # vendor/bullet/src/
                    vendor/glad/
                    # vendor/glfw/include/
                    vendor/glm/
                    )

file(GLOB VENDORS_SOURCES vendor/glad/glad.c)
file(GLOB PROJECT_HEADERS src/include/*.hh)
file(GLOB PROJECT_SOURCES src/*.cc src/*.c)
file(GLOB PROJECT_SHADERS shader/*.comp
                          shader/*.frag
                          shader/*.geom
                          shader/*.vert)

file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          Readme.md
                         .gitattributes
                         .gitignore
                         .gitmodules)

source_group("src/include" FILES ${PROJECT_HEADERS})
source_group("shader" FILES ${PROJECT_SHADERS})
source_group("src" FILES ${PROJECT_SOURCES})
source_group("vendor" FILES ${VENDORS_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS} ${PROJECT_CONFIGS}
                               ${VENDORS_SOURCES})
target_link_libraries(${PROJECT_NAME} assimp glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES}
#                      BulletDynamics BulletCollision LinearMath
                      )
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
