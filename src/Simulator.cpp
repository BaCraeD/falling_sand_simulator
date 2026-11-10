#include "Simulator.h"
#include <algorithm>
#include <chrono>

namespace {
// Color definitions (0xAABBGGRR in little-endian format)
constexpr uint32_t COLOR_EMPTY = 0xFF181514; // Dark charcoal
constexpr uint32_t COLOR_WALL  = 0xFF7A7570; // Slate gray

inline uint32_t makeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (static_cast<uint32_t>(a) << 24) |
           (static_cast<uint32_t>(b) << 16) |
           (static_cast<uint32_t>(g) << 8)  |
           (static_cast<uint32_t>(r));
}
} // namespace

Simulator::Simulator(int width, int height)
    : m_width(width)
    , m_height(height)
    , m_grid(static_cast<size_t>(width * height))
    , m_pixelBuffer(static_cast<size_t>(width * height), COLOR_EMPTY)
    , m_rng(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count()))
{
}

void Simulator::clear() {
    std::fill(m_grid.begin(), m_grid.end(), Cell{CellType::Empty, 0, 0});
    m_activeParticles = 0;
}

void Simulator::setCell(int x, int y, CellType type) {
    if (!inBounds(x, y)) return;
    
    Cell& cell = at(x, y);
    if (cell.type == type) return;

    cell.type = type;
    cell.variation = static_cast<uint8_t>(m_rng() % 30);
    cell.updatedFrame = m_currentFrame;
}

void Simulator::spawnBrush(int centerX, int centerY, int radius, CellType type) {
    int r2 = radius * radius;
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy <= r2) {
                int nx = centerX + dx;
                int ny = centerY + dy;
                if (!inBounds(nx, ny)) continue;

                // Density scatter for Sand/Water so it pours naturally
                if (type == CellType::Sand || type == CellType::Water) {
                    if ((m_rng() % 100) < 65) {
                        setCell(nx, ny, type);
                    }
                } else {
                    setCell(nx, ny, type);
                }
            }
        }
    }
}

void Simulator::swapCells(int x1, int y1, int x2, int y2) {
    Cell temp = at(x1, y1);
    at(x1, y1) = at(x2, y2);
    at(x2, y2) = temp;

    at(x1, y1).updatedFrame = m_currentFrame;
    at(x2, y2).updatedFrame = m_currentFrame;
}

void Simulator::updateSand(int x, int y) {
    int belowY = y + 1;
    if (belowY >= m_height) return;

    // 1. Fall directly down if empty or displace water (sand is denser than water)
    CellType belowType = at(x, belowY).type;
    if (belowType == CellType::Empty || belowType == CellType::Water) {
        swapCells(x, y, x, belowY);
        return;
    }

    // 2. Slide diagonally (randomize left vs right to prevent direction bias)
    int dir = (m_rng() % 2 == 0) ? -1 : 1;
    int firstX = x + dir;
    int secondX = x - dir;

    if (inBounds(firstX, belowY)) {
        CellType t = at(firstX, belowY).type;
        if (t == CellType::Empty || t == CellType::Water) {
            swapCells(x, y, firstX, belowY);
            return;
        }
    }

    if (inBounds(secondX, belowY)) {
        CellType t = at(secondX, belowY).type;
        if (t == CellType::Empty || t == CellType::Water) {
            swapCells(x, y, secondX, belowY);
            return;
        }
    }
}

void Simulator::updateWater(int x, int y) {
    int belowY = y + 1;

    // 1. Fall straight down
    if (belowY < m_height && at(x, belowY).type == CellType::Empty) {
        swapCells(x, y, x, belowY);
        return;
    }

    // 2. Diagonal slide
    int dir = (m_rng() % 2 == 0) ? -1 : 1;
    int firstX = x + dir;
    int secondX = x - dir;

    if (belowY < m_height && inBounds(firstX, belowY) && at(firstX, belowY).type == CellType::Empty) {
        swapCells(x, y, firstX, belowY);
        return;
    }
    if (belowY < m_height && inBounds(secondX, belowY) && at(secondX, belowY).type == CellType::Empty) {
        swapCells(x, y, secondX, belowY);
        return;
    }

    // 3. Fluid lateral dispersion (flow sideways)
    if (inBounds(firstX, y) && at(firstX, y).type == CellType::Empty) {
        swapCells(x, y, firstX, y);
        return;
    }
    if (inBounds(secondX, y) && at(secondX, y).type == CellType::Empty) {
        swapCells(x, y, secondX, y);
        return;
    }
}

void Simulator::step() {
    ++m_currentFrame;
    m_activeParticles = 0;

    // Update bottom-to-top to avoid double-processing falling particles
    for (int y = m_height - 1; y >= 0; --y) {
        // Alternate horizontal sweep direction to eliminate lateral bias
        bool leftToRight = (y + m_currentFrame) % 2 == 0;
        int xStart = leftToRight ? 0 : m_width - 1;
        int xEnd = leftToRight ? m_width : -1;
        int step = leftToRight ? 1 : -1;

        for (int x = xStart; x != xEnd; x += step) {
            Cell& cell = at(x, y);

            if (cell.type == CellType::Empty) continue;
            ++m_activeParticles;

            if (cell.updatedFrame == m_currentFrame) continue;

            switch (cell.type) {
                case CellType::Sand:
                    updateSand(x, y);
                    break;
                case CellType::Water:
                    updateWater(x, y);
                    break;
                case CellType::Wall:
                default:
                    break;
            }
        }
    }
}

const uint32_t* Simulator::getPixelData() {
    for (int i = 0; i < m_width * m_height; ++i) {
        const Cell& cell = m_grid[i];
        switch (cell.type) {
            case CellType::Empty:
                m_pixelBuffer[i] = COLOR_EMPTY;
                break;
            case CellType::Sand: {
                uint8_t r = static_cast<uint8_t>(std::min(255, 215 + cell.variation));
                uint8_t g = static_cast<uint8_t>(std::min(255, 175 + cell.variation));
                uint8_t b = static_cast<uint8_t>(90 + (cell.variation / 2));
                m_pixelBuffer[i] = makeColor(r, g, b);
                break;
            }
            case CellType::Water: {
                uint8_t r = 35;
                uint8_t g = static_cast<uint8_t>(115 + cell.variation);
                uint8_t b = static_cast<uint8_t>(215 + cell.variation);
                m_pixelBuffer[i] = makeColor(r, g, b);
                break;
            }
            case CellType::Wall: {
                uint8_t v = static_cast<uint8_t>(110 + cell.variation);
                m_pixelBuffer[i] = makeColor(v, v, v);
                break;
            }
        }
    }
    return m_pixelBuffer.data();
}
