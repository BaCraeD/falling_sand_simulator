# 2D Falling Sand Simulator

A real-time, interactive physics sandbox game built in modern C++ (C++20), **SDL3**, and **OpenGL**.

---

## What Does This App Do?

This app is a **falling sand toy / cellular automata simulation**:

- **Grid-based Physics:** The world is divided into a fine grid of thousands of tiny cells.
- **Sand:** Falls straight down due to gravity, slides down slopes into pyramids, and sinks in water.
- **Water:** Falls down, slides diagonally, and flows laterally across flat surfaces to fill hollows.
- **Walls:** Solid, indestructible barriers that you can draw to make ramps, funnels, or containers.
- **Real-time Interaction:** Use your mouse to draw or erase particles with varying brush sizes, just like digital spray paint.
- **GPU Accelerated:** Instead of drawing thousands of tiny rectangles on the CPU, the entire grid is uploaded to the graphics card as a texture and rendered smoothly with OpenGL shaders.

---

## Controls

| Action | Control |
|---|---|
| **Draw / Spawn particle** | **Left Click & Drag** |
| **Erase** | **Right Click & Drag** |
| **Change Brush Size** | **Mouse Scroll Wheel** or `[` / `]` |
| **Select Sand** | `1` |
| **Select Water** | `2` |
| **Select Wall** | `3` |
| **Select Eraser** | `4` |
| **Pause / Resume** | `Spacebar` |
| **Clear Screen** | `C` |
| **Exit** | `Escape` |

The window title bar displays real-time statistics: current FPS, total active particle count, selected tool, and brush radius.

---

## How to Compile & Run

### Prerequisites

Make sure you have installed:
- A C++20 compiler (`g++` 10+, `clang++` 11+, or MSVC 2019+)
- **CMake** (version 3.20 or newer)
- **Git** (used automatically by CMake to fetch SDL3 if not already installed on your system)
- **OpenGL development headers** (standard on all desktop operating systems)

#### Linux (Ubuntu / Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libgl1-mesa-dev
```

---

### Build Instructions

1. **Open your terminal** and navigate into the project directory:
   ```bash
   cd falling_sand_simulator
   ```

2. **Create a build folder and run CMake**:
   ```bash
   mkdir -p build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```
   *(Note: The first time you run this, CMake will automatically download and configure SDL3 for you if it isn't already installed on your machine.)*

3. **Compile the executable**:
   ```bash
   cmake --build . --config Release -j$(nproc)
   ```

4. **Launch the simulator**:
   ```bash
   ./falling_sand
   ```

---

### Windows (Visual Studio / Command Prompt)

```powershell
cd falling_sand_simulator
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
.\Release\falling_sand.exe
```

---

### macOS

```bash
cd falling_sand_simulator
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j$(sysctl -n hw.ncpu)
./falling_sand
```
