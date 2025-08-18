#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include "utility.hpp"
#include "honeycomb.hpp"

namespace cb {

Honeycomb::Honeycomb(sf::Vector2f cellSize, float gap, sf::Color colorOuter, sf::Color colorInner)
    :cellSize{cellSize}, gap{gap}, colorOuter{colorOuter}, colorInner{colorInner}
{
    setGlow(3.0f, 8,  sf::Color{colorOuter.r, colorOuter.g, colorOuter.b, 0x60});
    generateLayers();
}

void Honeycomb::generateLayers() 
{
    const float radius = cellSize.x * 0.5f;
    drawHexagons(radius);
}

void Honeycomb::appendGlowRing(float radius) 
{
    constexpr int sides = 6;

    for (int i = 0; i < glowLayers; ++i) 
    {
        const float r1 = radius + i * (glowRadius / glowLayers);
        const float r2 = radius + (i + 1) * (glowRadius / glowLayers);

        const float alpha = 1.f - static_cast<float>(i) / static_cast<float>(glowLayers);
        const sf::Color glow = sf::Color
        {
            glowColor.r,
            glowColor.g,
            glowColor.b,
            static_cast<uint8_t>(glowColor.a * alpha)
        };

        std::array<sf::Vector2f, sides> outer{};
        std::array<sf::Vector2f, sides> inner{};

        for (int j = 0; j < sides; ++j) 
        {
            const float angle = static_cast<float>(j) * 60.f + 30.0f;
            const float rad = sf::degrees(angle).asRadians();
            outer[j] = sf::Vector2f{std::cos(rad) * r2, std::sin(rad) * r2};
            inner[j] = sf::Vector2f{std::cos(rad) * r1, std::sin(rad) * r1};
        }

        for (int j = 0; j < sides; ++j) 
        {
            const int next = (j + 1) % sides;

            fill.append(sf::Vertex{inner[j],    glow});
            fill.append(sf::Vertex{outer[j],    glow});
            fill.append(sf::Vertex{outer[next], glow});

            fill.append(sf::Vertex{inner[j],    glow});
            fill.append(sf::Vertex{outer[next], glow});
            fill.append(sf::Vertex{inner[next], glow});
        }
    }
}

void Honeycomb::drawHexagons(float radius) 
{
    constexpr int sides = 6;
    const int q = static_cast<int>(std::floor(radius / gap));

    for (int i = 0; i < q; ++i) 
    {
        const float r = radius - i * gap;
        if (r <= 0.f) break;

        const float t = static_cast<float>(i) / static_cast<float>(q - 1);
        const sf::Color ringColor = lerpColor(colorOuter, colorInner, t);

        std::array<sf::Vector2f, sides> points{};

        for (int j = 0; j < sides; ++j) 
        {
            const float angle = static_cast<float>(j) * 60.f + 30.0f;
            const float rad = sf::degrees(angle).asRadians();
            points[j] = sf::Vector2f{std::cos(rad) * r, std::sin(rad) * r};
        }

        for (int j = 0; j < sides; ++j) 
        {
            const int next = (j + 1) % sides;
            fill.append(sf::Vertex{sf::Vector2f{ 0.0f, 0.0f }, backgroundColor});
            fill.append(sf::Vertex{points[j], backgroundColor});
            fill.append(sf::Vertex{points[next], backgroundColor});
        }

        for (int j = 0; j < sides; ++j) 
        {
            const int next = (j + 1) % sides;
            wire.append(sf::Vertex{points[j], ringColor});
            wire.append(sf::Vertex{points[next], ringColor});
        }
    }
    appendGlowRing(radius);
}


void Honeycomb::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform.translate(position);
    target.draw(fill, states);
    target.draw(wire, states);
}


}
