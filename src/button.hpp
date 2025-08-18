#pragma once 
#include <SFML/Graphics.hpp>
#include <functional>


class Button {
public:
    sf::Vector2f position;
    sf::Vector2f size;

    sf::Color normalColor   = sf::Color(100, 100, 100);
    sf::Color hoveredColor  = sf::Color(150, 150, 150);
    sf::Color clickedColor  = sf::Color(200, 200, 200);

    bool hovered = false;
    bool pressed = false;

    std::function<void()> onClick;

    Button(const sf::Vector2f& pos, const sf::Vector2f& sz)
        : position(pos), size(sz) {}

    void update(const sf::Vector2f& mousePos, bool mousePressed, bool mouseReleased)
    {
        hovered = contains(mousePos);

        if (hovered && mousePressed) {
            pressed = true;
        }

        if (pressed && hovered && mouseReleased) {
            pressed = false;
            if (onClick) onClick();
        }

        if (mouseReleased && !hovered) {
            pressed = false;
        }
    }

    void draw(sf::RenderTarget& target)
    {
        sf::RectangleShape rect(size);
        rect.setPosition(position);

        if (pressed) {
            rect.setFillColor(clickedColor);
        } else if (hovered) {
            rect.setFillColor(hoveredColor);
        } else {
            rect.setFillColor(normalColor);
        }

        target.draw(rect);
    }

    bool contains(const sf::Vector2f& point) const
    {
        return point.x >= position.x && point.x <= position.x + size.x &&
               point.y >= position.y && point.y <= position.y + size.y;
    }
};

