#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include <set>
#include "polyline.hpp"
#include "honeycomb.hpp"
#include "identicon.hpp"
#include "colours.hpp"
#include "levels.hpp"
#include "button.hpp"

namespace cb {

struct Node
{
    enum Type { Base, Intersection };
    uint32_t uid;
    float x;
    float y;
    Type type;
};

struct Chip
{
    uint32_t uid;
    int position; // current location
};

struct Colorset 
{
    sf::Color background    { hexColor(color::Material::Background) };
    sf::Color foreground    { hexColor(color::Material::Blue)       };
    sf::Color inactiveHint  { hexColor(color::Material::Disabled)   };
    sf::Color activeHint    { hexColor(color::Material::Blue)       };
    sf::Color path          { hexColor(color::Material::Purple)     };
    sf::Color selected      { hexColor(color::Material::Purple)     };
    sf::Color error         { hexColor(color::Material::Error)      };
};

struct DragState
{
    bool active = false;
    bool animating = false;
    int uid = -1;
    int origin = -1;
    int target = -1;                            // hovered sector
    float phase = 0.0f;                         // [0.0 ... 1.0]
    sf::Vector2f mousePosition;
    sf::Vector2f snapback;
    std::vector<int> path;
    std::vector<sf::Vector2f> route;    // actual positions
    sf::Vector2f offset;
};


class Board
{
    public:
        Board(const sf::Vector2f&);
        void draw(sf::RenderTarget&, float, bool, sf::Vector2f);
        void mouseDown(const sf::Vector2f&);
        void mouseMove(const sf::Vector2f&);
        void mouseUp();
        bool isDragging() const { return drag.active; };
        void loadLevel(const std::string&, bool);
    
    private:
        Chip* getChipByUid(int uid);
        void addPoint(uint32_t, int, int);
        void addConnection(int, int);
        void placeChip(uint32_t, int);
        void setTargetPositions(const std::vector<int>&);
        void bake();
        void bakeChipTextures(float cellSize = 48.0f);
        void bakeHintTextures(float cellSize = 48.0f);
        void drawChips(sf::RenderTarget&) const;
        void drawHints(sf::RenderTarget&) const;
        void drawDraggedChip(sf::RenderTarget&) const;
        std::vector<int> findPath(int start, int goal) const;
        void update(float dt);
        void clear();
        void loadFromStream(std::istream&);

        std::vector<Button> levelButtons;
        sf::Font uiFont;
        const sf::Vector2f wsize;
        float chipScale = 1.0f;
        float hintScale = 1.0f;
        Colorset colorset;
        std::vector<int> targetPositions;
        std::vector<PolyLine> bakedConnections;
        std::vector<Honeycomb> bakedHoneycombs;
        std::unordered_map<uint32_t, Node> node;
        std::unordered_map<uint32_t, std::vector<uint32_t>> adjacency;
        std::vector<Chip> chips;
        std::unordered_map<int, std::shared_ptr<sf::Texture>> chipTextures;
        std::unordered_map<int, std::shared_ptr<sf::Texture>> hintTextures;

        DragState drag;
};


}
