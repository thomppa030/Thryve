cmake_minimum_required(VERSION 3.27)
project(Thryve)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

include_directories(config)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Define the path to the shaders directory relative to this CMakeLists.txt
set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThryveRenderer/shaders")
set(RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ThryveRenderer/resources")
set(PROFILE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Profiling/ProfilingData")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Config.h.in ${CMAKE_BINARY_DIR}/generated/Config.h)

include_directories(${CMAKE_BINARY_DIR}/generated)


add_subdirectory(external/glfw)
add_subdirectory(external/spdlog)
add_subdirectory(external/json)

include_directories(external/spdlog/include)
include_directories(external/glfw/include)
include_directories(external/json/include)

add_subdirectory(ThryveRenderer)

include_directories(Vulkan PUBLIC ${Vulkan_INCLUDE_DIRS})

# Define ImGui as a library target
add_library(imgui STATIC
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_vulkan.cpp
)

# Include directories for the ImGui target
target_include_directories(imgui
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include
        ${Vulkan_INCLUDE_DIRS}
)

# Now, you should also provide an alias for imgui to not collide with system-installed libraries, like so:
add_library(Thryve::imgui ALIAS imgui)
add_executable(Thryve main.cpp
        ThryveApplication.cpp
        ThryveApplication.h
        GraphicsContext.h
        IRenderContext.h
        ThryveRenderer/src/pch.cpp)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES} ThryveRenderer glfw glm::glm spdlog::spdlog imgui nlohmann_json::nlohmann_json)

