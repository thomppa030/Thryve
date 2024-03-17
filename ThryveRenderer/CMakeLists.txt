cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(ThryveRenderer LANGUAGES CXX)

# Set the C++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use modern CMake practices for finding and linking to libraries
find_package(Vulkan REQUIRED)

# Precompiled header
set(PRECOMPILE_HEADER "include/pch.h")

# Define the library target
add_library(ThryveRenderer
        include/Vulkan/VulkanTextureImage.cpp
        include/Vulkan/VulkanTextureImage.h
        include/utils/ImageUtils.h)
target_precompile_headers(ThryveRenderer PRIVATE ${PRECOMPILE_HEADER})

# Include directories for the ThryveRenderer target
target_include_directories(ThryveRenderer
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${Vulkan_INCLUDE_DIRS}
        external/glm
        external/stb
)

# Source and header files for the ThryveRenderer target
file(GLOB_RECURSE SOURCES "src/*cpp")
file(GLOB_RECURSE HEADERS "include/*h")
target_sources(ThryveRenderer PRIVATE ${SOURCES} ${HEADERS})

# Link libraries to the ThryveRenderer target
target_link_libraries(ThryveRenderer
        PRIVATE
        glfw
        glm::glm
        spdlog::spdlog
)

# Add external dependencies as subdirectories
add_subdirectory(external/glm)


# Catch2 is header-only; consider using FetchContent or add it to the target_include_directories where it's used.
# If Catch2 is used for testing, you might want to conditionally include it:
# if(THRYVE_BUILD_TESTS)
#     include_directories(external/Catch2/single_include)
# endif()

# ... Rest of your CMake code for installing, testing, etc.
