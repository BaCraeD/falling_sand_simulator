#pragma once

#include <vector>
#include <cstdint>
#include <random>

enum class CellType : uint8_t {
    Empty = 0,
    Sand,
    Water,
    Wall
};

struct Cell {
    CellType type{CellType::Empty};
    uint8_t variation{0};    // Subtle visual shade variation
    uint8_t updatedFrame{0}; // Prevents cells from updating multiple times in one tick
};

class Simulator {
public:
    Simulator(int width, int height);

    void step();
    void clear();

    void setCell(int x, int y, CellType type);
    void spawnBrush(int centerX, int centerY, int radius, CellType type);

    int getWidth() const noexcept { return m_width; }
    int getHeight() const noexcept { return m_height; }
    size_t getParticleCount() const noexcept { return m_activeParticles; }

    // Returns a pointer to raw RGBA32 pixels for OpenGL texture streaming
    const uint32_t* getPixelData();

private:
    bool inBounds(int x, int y) const noexcept {
        return x >= 0 && x < m_width && y >= 0 && y < m_height;
    }

    Cell& at(int x, int y) { return m_grid[y * m_width + x]; }
    const Cell& at(int x, int y) const { return m_grid[y * m_width + x]; }

    void swapCells(int x1, int y1, int x2, int y2);
    void updateSand(int x, int y);
    void updateWater(int x, int y);

    int m_width;
    int m_height;
    uint8_t m_currentFrame{0};
    size_t m_activeParticles{0};

    std::vector<Cell> m_grid;
    std::vector<uint32_t> m_pixelBuffer; // Pre-allocated RGBA8 texture upload buffer

    std::mt19937 m_rng;
};
