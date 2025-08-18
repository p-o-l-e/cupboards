#pragma once
#include <SFML/Graphics.hpp>
#include <bitset>
#include <array>
#include <random>
#include <cmath>
#include "colours.hpp"
#define M_PI 3.14159265358979323846

namespace cb {

template <size_t Size>
std::array<std::bitset<Size>, Size> generateIdenticon(unsigned seed)
{
    std::array<std::bitset<Size>, Size> grid;

    constexpr size_t half = Size / 2 + (Size % 2);

    auto hash = [](uint32_t x) -> uint32_t {
        x = (x ^ 61) ^ (x >> 16);
        x = x + (x << 3);
        x = x ^ (x >> 4);
        x = x * 0x27d4eb2d;
        x = x ^ (x >> 15);
        return x;
    };

    for (size_t row = 0; row < Size; ++row)
    {
        for (size_t col = 0; col < half; ++col)
        {
            uint32_t h = hash(seed ^ (row * 92837111) ^ (col * 689287499));
            bool value = (h >> (col % 16)) & 1;

            grid[row][col] = value;
            grid[row][Size - col - 1] = value;
        }
    }

    return grid;
}



template <size_t Size>
void drawIdenticon(sf::RenderTarget& target,
                   const std::array<std::bitset<Size>, Size>& grid,
                   sf::Vector2f position,
                   float circleDiameter,
                   float scaleFactor = 3.0f,            // 1 = scale factor of circle
                   sf::Color fillColor = sf::Color::White,
                   sf::Color backgroundColor = hexColor(color::Material::Background),
                   bool bg = true)
{
    const float circleRadius = circleDiameter / 2.0f;
    const sf::Vector2f center{position.x + circleRadius, position.y + circleRadius};
    
    if(bg)
    {

        // Background
        sf::CircleShape circle(circleRadius);
        circle.setFillColor(hexColor(color::Material::Accent));
        circle.setOrigin(sf::Vector2f{circleRadius, circleRadius});
        circle.setPosition(center);
        target.draw(circle);
    }
    // Compute size
    const float identiconSize = circleDiameter / scaleFactor;
    const float cellSize = identiconSize / Size;
    const sf::Vector2f offset
    {
        center.x - identiconSize / 2.0f,
        center.y - identiconSize / 2.0f
    };
    // Draw identicon cells
    sf::RectangleShape cell{sf::Vector2f{cellSize, cellSize}};
    cell.setFillColor(fillColor);

    for (size_t row = 0; row < Size; ++row)
    {
        for (size_t col = 0; col < Size; ++col)
        {
            if (!grid[row][col])
            {
                continue;
            }

            sf::Vector2f cellPos
            {
                offset.x + col * cellSize,
                offset.y + row * cellSize
            };

            cell.setPosition(cellPos);
            target.draw(cell);
        }
    }
    // Glowing ring
    if(bg)
    {
        sf::VertexArray glow{sf::PrimitiveType::TriangleStrip};
        const int segments = 64;
        const float glowThickness = circleDiameter * 0.2f; // adjust for softness

        sf::Color innerColor{ hexColor(color::Material::Purple) }; // purple glow
        sf::Color outerColor{innerColor};
        outerColor.a = 0;
        auto glowRadius = circleRadius * 0.9f;
        for (int i = 0; i <= segments; ++i)
        {
            float angle = i * 2.0f * M_PI / segments;
            sf::Vector2f dir{std::cos(angle), std::sin(angle)};
            sf::Vector2f inner = center + dir * glowRadius;
            sf::Vector2f outer = center + dir * (glowRadius + glowThickness);

            glow.append(sf::Vertex{inner, innerColor});
            glow.append(sf::Vertex{outer, outerColor});
        }

        target.draw(glow);
    }
}



template <size_t Size>
std::shared_ptr<sf::Texture> bakeIdenticonTexture(unsigned seed, float diameter, bool bg = true, sf::Color id_color = hexColor(color::Material::Green))
{
const unsigned imageSize = static_cast<unsigned>(Size * diameter);

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;

    sf::RenderTexture renderTexture({ imageSize, imageSize }, settings);
    renderTexture.clear(sf::Color::Transparent);

    auto grid = generateIdenticon<Size>(seed);
    const float radius = diameter * 0.5f;

    sf::Vector2f centerPos
    {
        std::floor(imageSize * 0.5f - radius),
        std::floor(imageSize * 0.5f - radius)
    };

    drawIdenticon(
        renderTexture,
        grid,
        centerPos,
        diameter,
        1.7f,
        id_color,
        hexColor(color::Material::Background),
        bg
    );

    renderTexture.display();

    auto texture = std::make_shared<sf::Texture>(renderTexture.getTexture());
    texture->setSmooth(true);

    return texture;}

}
