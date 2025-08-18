#pragma once
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <algorithm>

constexpr sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t) 
{
    return sf::Color
    {
        static_cast<uint8_t>(a.r + t * (b.r - a.r)),
        static_cast<uint8_t>(a.g + t * (b.g - a.g)),
        static_cast<uint8_t>(a.b + t * (b.b - a.b)),
        static_cast<uint8_t>(a.a + t * (b.a - a.a))
    };
}

inline float distance(const sf::Vector2f& a, const sf::Vector2f& b)
{
    sf::Vector2f delta = a - b;
    return std::sqrt(delta.x * delta.x + delta.y * delta.y);
}

constexpr sf::Vector2f lerp(const sf::Vector2f& a, const sf::Vector2f& b, float t)
{
    return a + (b - a) * t;
}

inline float easeOutBounce(float t)
{
    return std::sqrt(1.0f - std::pow(t - 1.0f, 2));
}
