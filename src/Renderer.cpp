#include "Renderer.h"
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// Modern OpenGL 3.3 Core function pointer declarations
namespace {
PFNGLCREATESHADERPROC            glCreateShader            = nullptr;
PFNGLSHADERSOURCEPROC            glShaderSource            = nullptr;
PFNGLCOMPILESHADERPROC           glCompileShader           = nullptr;
PFNGLGETSHADERIVPROC             glGetShaderiv             = nullptr;
PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog        = nullptr;
PFNGLCREATEPROGRAMPROC           glCreateProgram           = nullptr;
PFNGLATTACHSHADERPROC            glAttachShader            = nullptr;
PFNGLLINKPROGRAMPROC             glLinkProgram             = nullptr;
PFNGLGETPROGRAMIVPROC            glGetProgramiv            = nullptr;
PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog       = nullptr;
PFNGLUSEPROGRAMPROC              glUseProgram              = nullptr;
PFNGLDELETESHADERPROC            glDeleteShader            = nullptr;
PFNGLDELETEPROGRAMPROC           glDeleteProgram           = nullptr;
PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays         = nullptr;
PFNGLBINDVERTEXARRAYPROC         glBindVertexArray         = nullptr;
PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays      = nullptr;
PFNGLGENBUFFERSPROC              glGenBuffers              = nullptr;
PFNGLBINDBUFFERPROC              glBindBuffer              = nullptr;
PFNGLBUFFERDATAPROC              glBufferData              = nullptr;
PFNGLDELETEBUFFERSPROC           glDeleteBuffers           = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer     = nullptr;
PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation      = nullptr;
PFNGLUNIFORM1IPROC               glUniform1i               = nullptr;

bool loadGLFunctions() {
    #define LOAD_PROC(type, name) \
        name = reinterpret_cast<type>(SDL_GL_GetProcAddress(#name)); \
        if (!name) { \
            std::cerr << "Failed to load OpenGL function: " << #name << "\n"; \
            return false; \
        }

    LOAD_PROC(PFNGLCREATESHADERPROC,            glCreateShader);
    LOAD_PROC(PFNGLSHADERSOURCEPROC,            glShaderSource);
    LOAD_PROC(PFNGLCOMPILESHADERPROC,           glCompileShader);
    LOAD_PROC(PFNGLGETSHADERIVPROC,             glGetShaderiv);
    LOAD_PROC(PFNGLGETSHADERINFOLOGPROC,        glGetShaderInfoLog);
    LOAD_PROC(PFNGLCREATEPROGRAMPROC,           glCreateProgram);
    LOAD_PROC(PFNGLATTACHSHADERPROC,            glAttachShader);
    LOAD_PROC(PFNGLLINKPROGRAMPROC,             glLinkProgram);
    LOAD_PROC(PFNGLGETPROGRAMIVPROC,            glGetProgramiv);
    LOAD_PROC(PFNGLGETPROGRAMINFOLOGPROC,       glGetProgramInfoLog);
    LOAD_PROC(PFNGLUSEPROGRAMPROC,              glUseProgram);
    LOAD_PROC(PFNGLDELETESHADERPROC,            glDeleteShader);
    LOAD_PROC(PFNGLDELETEPROGRAMPROC,           glDeleteProgram);
    LOAD_PROC(PFNGLGENVERTEXARRAYSPROC,         glGenVertexArrays);
    LOAD_PROC(PFNGLBINDVERTEXARRAYPROC,         glBindVertexArray);
    LOAD_PROC(PFNGLDELETEVERTEXARRAYSPROC,      glDeleteVertexArrays);
    LOAD_PROC(PFNGLGENBUFFERSPROC,              glGenBuffers);
    LOAD_PROC(PFNGLBINDBUFFERPROC,              glBindBuffer);
    LOAD_PROC(PFNGLBUFFERDATAPROC,              glBufferData);
    LOAD_PROC(PFNGLDELETEBUFFERSPROC,           glDeleteBuffers);
    LOAD_PROC(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);
    LOAD_PROC(PFNGLVERTEXATTRIBPOINTERPROC,     glVertexAttribPointer);
    LOAD_PROC(PFNGLGETUNIFORMLOCATIONPROC,      glGetUniformLocation);
    LOAD_PROC(PFNGLUNIFORM1IPROC,               glUniform1i);
    #undef LOAD_PROC

    return true;
}

std::string readFileOrDefault(const std::string& path, const std::string& fallback) {
    std::ifstream file(path);
    if (!file.is_open()) return fallback;
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
} // namespace

Renderer::Renderer(const char* title, int windowWidth, int windowHeight, int simWidth, int simHeight)
    : m_windowWidth(windowWidth)
    , m_windowHeight(windowHeight)
    , m_simWidth(simWidth)
    , m_simHeight(simHeight)
{
    if (!initSDL(title, windowWidth, windowHeight)) return;
    if (!initOpenGL()) return;
    setupQuad();
    m_initialized = true;
}

Renderer::~Renderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_textureID) glDeleteTextures(1, &m_textureID);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
    if (m_glContext) SDL_GL_DestroyContext(m_glContext);
    if (m_window) SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Renderer::initSDL(const char* title, int w, int h) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_window = SDL_CreateWindow(
        title,
        w,
        h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (!m_window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << "\n";
        return false;
    }

    SDL_GL_MakeCurrent(m_window, m_glContext);
    SDL_GL_SetSwapInterval(1); // Enable VSync

    return true;
}

bool Renderer::initOpenGL() {
    if (!loadGLFunctions()) return false;

    if (!loadShaders("shaders/vertex.glsl", "shaders/fragment.glsl")) {
        std::cerr << "Shader compilation failed.\n";
        return false;
    }

    // Create texture for the simulation grid
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_simWidth, m_simHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool Renderer::loadShaders(const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string defaultVS = R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
})";

    const std::string defaultFS = R"(#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
uniform sampler2D uTexture;
void main() {
    FragColor = texture(uTexture, TexCoord);
})";

    std::string vsCode = readFileOrDefault(vertexPath, defaultVS);
    std::string fsCode = readFileOrDefault(fragmentPath, defaultFS);

    const char* vSrc = vsCode.c_str();
    const char* fSrc = fsCode.c_str();

    auto compileShader = [](unsigned int type, const char* src) -> unsigned int {
        unsigned int id = glCreateShader(type);
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int success;
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(id, 512, nullptr, log);
            std::cerr << "Shader compile log:\n" << log << "\n";
        }
        return id;
    };

    unsigned int vs = compileShader(GL_VERTEX_SHADER, vSrc);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fSrc);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vs);
    glAttachShader(m_shaderProgram, fs);
    glLinkProgram(m_shaderProgram);

    int success;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(m_shaderProgram, 512, nullptr, log);
        std::cerr << "Program link log:\n" << log << "\n";
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(m_shaderProgram);
    int texLoc = glGetUniformLocation(m_shaderProgram, "uTexture");
    glUniform1i(texLoc, 0);

    return true;
}

void Renderer::setupQuad() {
    // Two triangles forming a full-screen quad (Position vec2, UV vec2)
    // Texture V is flipped (1.0 at top, 0.0 at bottom) to match grid coordinate row 0
    float quadVertices[] = {
        // PosX,  PosY,  TexU, TexV
        -1.0f,  1.0f,  0.0f, 0.0f,
        -1.0f, -1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 1.0f,

        -1.0f,  1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 0.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Positions (Location 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Texture Coords (Location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Renderer::updateTexture(const uint32_t* pixelData) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_simWidth, m_simHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
}

void Renderer::render() {
    int w, h;
    SDL_GetWindowSizeInPixels(m_window, &w, &h);
    glViewport(0, 0, w, h);

    glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::swapBuffers() {
    SDL_GL_SwapWindow(m_window);
}

void Renderer::screenToGrid(float screenX, float screenY, int& gridX, int& gridY) const {
    int winW, winH;
    SDL_GetWindowSize(m_window, &winW, &winH);
    if (winW <= 0 || winH <= 0) return;

    float normX = screenX / static_cast<float>(winW);
    float normY = screenY / static_cast<float>(winH);

    gridX = static_cast<int>(normX * static_cast<float>(m_simWidth));
    gridY = static_cast<int>(normY * static_cast<float>(m_simHeight));
}

void Renderer::getWindowSize(int& w, int& h) const {
    SDL_GetWindowSize(m_window, &w, &h);
}
