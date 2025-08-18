
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include "board.hpp"
#include "colours.hpp"
#include "levels.hpp"

int main(int argc, char* argv[])
{
    std::cout << "Vendor:   " << glGetString(GL_VENDOR) << "\n";
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "Version:  " << glGetString(GL_VERSION) << "\n";

    sf::Vector2f wsize { 600.0f, 600.0f };
    cb::Board board(wsize);
    if(argc > 1)
    {
        board.loadLevel(argv[1], true);
    }
    else
    {
        board.loadLevel(level3, false);
    }

    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode({ static_cast<unsigned>(wsize.x), static_cast<unsigned>(wsize.y) }), "Cupboards", sf::Style::Titlebar, sf::State::Windowed, settings);
    window.setFramerateLimit(144);

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* e = event->getIf<sf::Event::MouseButtonPressed>())
            {
                if (e->button == sf::Mouse::Button::Left)
                {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    board.mouseDown(mousePos);
                }
            }
            else if (const auto* e = event->getIf<sf::Event::MouseButtonReleased>())
            {
                if (e->button == sf::Mouse::Button::Left)
                {
                    board.mouseUp();
                }
            }
            else if (event->is<sf::Event::MouseMoved>())
            {
                if (board.isDragging())
                {
                    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    board.mouseMove(mousePos);
                }
            }
        }

	    sf::Vector2f mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        window.clear(hexColor(color::Material::Background));

	    float scale = 48.0f/240.0f;
        board.draw(window, scale, false, mp);
        window.display();
    }
}

