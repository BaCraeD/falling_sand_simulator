#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <cstdint>

class Renderer {
public:
    Renderer(const char* title, int windowWidth, int windowHeight, int simWidth, int simHeight);
    ~Renderer();

    // Non-copyable
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool isInitialized() const noexcept { return m_initialized; }

    void updateTexture(const uint32_t* pixelData);
    void render();
    void swapBuffers();

    void screenToGrid(float screenX, float screenY, int& gridX, int& gridY) const;
    void getWindowSize(int& w, int& h) const;

    SDL_Window* getWindow() const noexcept { return m_window; }

private:
    bool initSDL(const char* title, int w, int h);
    bool initOpenGL();
    bool loadShaders(const std::string& vertexPath, const std::string& fragmentPath);
    void setupQuad();

    SDL_Window* m_window{nullptr};
    SDL_GLContext m_glContext{nullptr};

    int m_windowWidth{0};
    int m_windowHeight{0};
    int m_simWidth{0};
    int m_simHeight{0};

    unsigned int m_textureID{0};
    unsigned int m_vao{0};
    unsigned int m_vbo{0};
    unsigned int m_shaderProgram{0};

    bool m_initialized{false};
};
