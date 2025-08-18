#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <vector>
#include <cmath>
#include "utility.hpp"


namespace cb {

class PolyLine : public sf::Drawable
{
public:
    PolyLine(
        const sf::Vector2f& start,
        const sf::Vector2f& end,
        int thickness,
        int step,
        sf::Color colorInner = sf::Color::White,
        sf::Color colorOuter = sf::Color::Blue)
        : colorInner(colorInner), colorOuter(colorOuter), width(thickness)
    {
        generate(start, end, step);
    }

private:
    sf::VertexArray vertices{ sf::PrimitiveType::TriangleStrip };
    sf::Color colorInner;
    sf::Color colorOuter;
    float width;

    void generate(const sf::Vector2f& start, const sf::Vector2f& end, int step)
    {
        sf::Vector2f direction = end - start;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length == 0.f) return;

        sf::Vector2f unitDir = direction / length;
        sf::Vector2f normal(-unitDir.y, unitDir.x);

        int numLines = static_cast<int>(width / step);
        float halfWidth = step * numLines / 2.f;

        vertices.clear();
        vertices.setPrimitiveType(sf::PrimitiveType::Lines);

        for (int i = 0; i <= numLines; ++i)
        {
            float offset = -halfWidth + i * step;
            sf::Vector2f shift = normal * offset;

            sf::Vector2f p1 = start + shift;
            sf::Vector2f p2 = end + shift;

            float t = std::abs(offset) / halfWidth;
            sf::Color color = lerpColor(colorInner, colorOuter, t);

            vertices.append(sf::Vertex{ p1, color });
            vertices.append(sf::Vertex{ p2, color });
        }    
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override
    {
        target.draw(vertices, states);
    }
};
}
