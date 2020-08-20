cmake_minimum_required(VERSION 3.16)

project(VulkanPlayground
  LANGUAGES CXX CSharp
  VERSION 1.0.0
)
set(CMAKE_CXX_STANDARD 17)

include(.tools/sources.cmake)

add_subdirectory(deps/glm)
add_subdirectory(deps/glfw)
add_subdirectory(deps/tinyobjloader)
add_subdirectory(Render3d)
add_subdirectory(Tutorial)
