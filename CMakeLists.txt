cmake_minimum_required(VERSION 3.20)
project(FallingSandSimulator CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable optimizations in Release builds
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Find or Fetch SDL3
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
    message(STATUS "System SDL3 not detected. Fetching SDL3 via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-3.2.4
        GIT_SHALLOW TRUE
    )
    # Disable unnecessary SDL3 subcomponents to speed up compile time
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Executable target
add_executable(falling_sand
    src/main.cpp
    src/Simulator.cpp
    src/Renderer.cpp
)

target_include_directories(falling_sand PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(falling_sand PRIVATE
    SDL3::SDL3
    OpenGL::GL
)

# Copy shaders to the build directory so relative path loading always works
add_custom_command(TARGET falling_sand POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:falling_sand>/shaders
)
