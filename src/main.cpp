#include "Simulator.h"
#include "Renderer.h"

#include <iostream>
#include <string>
#include <chrono>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Simulation grid dimensions (High resolution cellular grid)
    constexpr int SIM_WIDTH = 320;
    constexpr int SIM_HEIGHT = 240;

    // Initial window size
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 960;

    Renderer renderer("2D Falling Sand Simulator [SDL3 + OpenGL]", WINDOW_WIDTH, WINDOW_HEIGHT, SIM_WIDTH, SIM_HEIGHT);
    if (!renderer.isInitialized()) {
        std::cerr << "Initialization failed. Exiting.\n";
        return 1;
    }

    Simulator sim(SIM_WIDTH, SIM_HEIGHT);

    // Initial obstacle boundaries
    for (int x = 0; x < SIM_WIDTH; ++x) {
        sim.setCell(x, SIM_HEIGHT - 1, CellType::Wall);
    }
    for (int y = 0; y < SIM_HEIGHT; ++y) {
        sim.setCell(0, y, CellType::Wall);
        sim.setCell(SIM_WIDTH - 1, y, CellType::Wall);
    }

    bool running = true;
    bool paused = false;
    CellType currentTool = CellType::Sand;
    int brushRadius = 5;

    bool isMouseDownLeft = false;
    bool isMouseDownRight = false;
    float mouseX = 0.0f;
    float mouseY = 0.0f;

    // Fixed timestep physics loop: 60 ticks per second
    constexpr double TIME_STEP = 1.0 / 60.0;
    using Clock = std::chrono::steady_clock;
    auto previousTime = Clock::now();
    double accumulator = 0.0;

    // FPS calculation
    int frameCount = 0;
    double fpsTimer = 0.0;

    while (running) {
        auto currentTime = Clock::now();
        double frameDuration = std::chrono::duration<double>(currentTime - previousTime).count();
        previousTime = currentTime;

        // Cap accumulator to avoid spiral of death
        if (frameDuration > 0.2) frameDuration = 0.2;
        accumulator += frameDuration;
        fpsTimer += frameDuration;
        frameCount++;

        // 1. Process SDL3 events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;

                case SDL_EVENT_KEY_DOWN: {
                    switch (event.key.key) {
                        case SDLK_ESCAPE:
                            running = false;
                            break;
                        case SDLK_SPACE:
                            paused = !paused;
                            break;
                        case SDLK_C:
                            sim.clear();
                            break;
                        case SDLK_1:
                            currentTool = CellType::Sand;
                            break;
                        case SDLK_2:
                            currentTool = CellType::Water;
                            break;
                        case SDLK_3:
                            currentTool = CellType::Wall;
                            break;
                        case SDLK_4:
                            currentTool = CellType::Empty;
                            break;
                        case SDLK_LEFTBRACKET:
                            brushRadius = std::max(1, brushRadius - 1);
                            break;
                        case SDLK_RIGHTBRACKET:
                            brushRadius = std::min(30, brushRadius + 1);
                            break;
                        default:
                            break;
                    }
                    break;
                }

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)  isMouseDownLeft = true;
                    if (event.button.button == SDL_BUTTON_RIGHT) isMouseDownRight = true;
                    mouseX = event.button.x;
                    mouseY = event.button.y;
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_LEFT)  isMouseDownLeft = false;
                    if (event.button.button == SDL_BUTTON_RIGHT) isMouseDownRight = false;
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    break;

                case SDL_EVENT_MOUSE_WHEEL:
                    if (event.wheel.y > 0) brushRadius = std::min(30, brushRadius + 1);
                    if (event.wheel.y < 0) brushRadius = std::max(1, brushRadius - 1);
                    break;

                default:
                    break;
            }
        }

        // 2. Mouse interactions (spawn or erase particles)
        if (isMouseDownLeft || isMouseDownRight) {
            int gx, gy;
            renderer.screenToGrid(mouseX, mouseY, gx, gy);
            CellType spawnType = isMouseDownRight ? CellType::Empty : currentTool;
            sim.spawnBrush(gx, gy, brushRadius, spawnType);
        }

        // 3. Fixed timestep physics update
        while (accumulator >= TIME_STEP) {
            if (!paused) {
                sim.step();
            }
            accumulator -= TIME_STEP;
        }

        // 4. Render frame
        renderer.updateTexture(sim.getPixelData());
        renderer.render();
        renderer.swapBuffers();

        // 5. Update title stats once per second
        if (fpsTimer >= 1.0) {
            std::string toolName;
            switch (currentTool) {
                case CellType::Sand:  toolName = "Sand [1]";  break;
                case CellType::Water: toolName = "Water [2]"; break;
                case CellType::Wall:  toolName = "Wall [3]";  break;
                case CellType::Empty: toolName = "Eraser [4]"; break;
            }

            std::string title = "Falling Sand | FPS: " + std::to_string(frameCount) +
                                " | Particles: " + std::to_string(sim.getParticleCount()) +
                                " | Tool: " + toolName +
                                " | Brush: " + std::to_string(brushRadius) +
                                (paused ? " [PAUSED]" : "");

            SDL_SetWindowTitle(renderer.getWindow(), title.c_str());
            frameCount = 0;
            fpsTimer = 0.0;
        }
    }

    return 0;
}
