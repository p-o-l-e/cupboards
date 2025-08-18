#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include "utility.hpp"


namespace cb {

class Honeycomb: public sf::Drawable 
{
    public:
        Honeycomb(sf::Vector2f, float, sf::Color, sf::Color);
        void setPosition(sf::Vector2f pos) { position = pos; }
        void setGlow(float r, int l, const sf::Color& col) { glowRadius = r; glowLayers = l; glowColor = col; };
        sf::Vector2f getPosition() const { return position; }

    private:
        sf::Vector2f cellSize;
        float gap;
        sf::Color colorOuter;
        sf::Color colorInner;
        sf::Vector2f position{};
        sf::Color backgroundColor = sf::Color{0x26, 0x32, 0x38, 0xFF};
        float glowRadius = 8.0f;
        int glowLayers = 16;
        sf::Color glowColor;

        sf::VertexArray fill{sf::PrimitiveType::Triangles};
        sf::VertexArray wire{sf::PrimitiveType::Lines};

        void generateLayers();
        void appendGlowRing(float);
        void drawHexagons(float);
        void draw(sf::RenderTarget&, sf::RenderStates ) const override;
};

}
