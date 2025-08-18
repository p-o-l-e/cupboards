#include "board.hpp"
#include <cstdint>
#include <iostream>
#include "colours.hpp"

namespace cb {

Board::Board(const sf::Vector2f& wsize): wsize(wsize) 
{
    levelButtons = {
        Button({20.f, 20.f},  {15.0f, 15.0f}),
        Button({50.f, 20.f},  {15.0f, 15.0f}),
        Button({80.f, 20.f},  {15.0f, 15.0f})
    };

    levelButtons[0].onClick = [&]() { loadLevel(level1, false); };
    levelButtons[1].onClick = [&]() { loadLevel(level2, false); };
    levelButtons[2].onClick = [&]() { loadLevel(level3, false); };

    levelButtons[0].normalColor = hexColor(color::Material::Green);
    levelButtons[1].normalColor = hexColor(color::Material::Yellow);
    levelButtons[2].normalColor = hexColor(color::Material::Red);
};

Chip* Board::getChipByUid(int uid)
{
    for(Chip& chip : chips)
    {
        if(chip.uid == uid) return &chip;
    }
    return nullptr;
}

void Board::bake()
{
    bakedConnections.clear();
    bakedHoneycombs.clear();
    // Connections
    {
        std::set<std::pair<int, int>> bakedPairs;
        for (const auto& [from, neighbors] : adjacency)
        {
            for (auto to : neighbors)
            {
                auto connection = std::minmax(from, to);
                if (bakedPairs.count(connection)) continue;

                bakedPairs.insert(connection);

                const Node& p1 = node.at(connection.first);
                const Node& p2 = node.at(connection.second);

                PolyLine line
                (
                    sf::Vector2f(p1.x, p1.y),
                    sf::Vector2f(p2.x, p2.y),
                    8, // thickness
                    4, // step
                    colorset.foreground,
                    colorset.background
                );
                bakedConnections.push_back(std::move(line));
            }
        }
    }

    // Base nodes
    for (const auto& [id, pt] : node)
    {
        if (pt.type == Node::Base)
        {
            Honeycomb honeycomb
            (
                sf::Vector2f{68.f, 68.f}, // cell size
                4.0f,                     // gap
                colorset.foreground,
                colorset.background
            );
            honeycomb.setPosition(sf::Vector2f{pt.x, pt.y});
            bakedHoneycombs.push_back(std::move(honeycomb));
        }
    }

    // Intersections
    for (const auto& [id, pt] : node)
    {
        if (pt.type == Node::Intersection)
        {
            Honeycomb honeycomb
            (
                sf::Vector2f{48.f, 48.f}, // cell size
                4.0f,                     // gap
                colorset.foreground,
                colorset.background
            );
            honeycomb.setPosition(sf::Vector2f{pt.x, pt.y});
            bakedHoneycombs.push_back(std::move(honeycomb));
        }
    }

    bakeChipTextures();
    bakeHintTextures();
}


void Board::addPoint(uint32_t uid, int x, int y)
{
    node[uid] = Node{ uid, static_cast<float>(x), static_cast<float>(y) };
}

void Board::addConnection(int from, int to)
{
    adjacency[from].push_back(to);
    adjacency[to].push_back(from); // bidirectional
}

void Board::placeChip(uint32_t chipId, int pointId)
{
    const Node& pt = node.at(pointId);
    Chip chip{ chipId, pointId };
    chips.push_back(chip);
}

void Board::setTargetPositions(const std::vector<int>& targets)
{
    targetPositions = targets;
}

void Board::bakeChipTextures(float cellSize)
{
    chipTextures.clear();
    const int texSize{static_cast<int>(cellSize * 8.0f)};

    for (const Chip& chip : chips)
    {
        auto texture = bakeIdenticonTexture<5>(chip.uid, cellSize);
        chipTextures[chip.uid] = texture;
    }
}

void Board::bakeHintTextures(float cellSize)
{
    for (std::size_t i = 0; i < chips.size(); ++i)
    {
        if (i >= targetPositions.size()) break;
        auto texture = bakeIdenticonTexture<5>
        (
            chips[i].uid,
            cellSize,
            false,
            sf::Color::White
        );
        hintTextures[chips[i].uid] = texture;
    }
}

void Board::drawChips(sf::RenderTarget& target) const
{
    float scale = 1.0f;
    for (const Chip& chip : chips)
    {
        if (drag.animating && chip.uid == drag.uid) continue;
        if (drag.active && !drag.animating && chip.uid == drag.uid) continue;

        auto position = node.find(chip.position);
        if(position == node.end()) continue;

        auto texture = chipTextures.find(chip.uid);
        if (texture == chipTextures.end()) continue;

        sf::Sprite sprite{*texture->second};

        sf::FloatRect bounds{ sprite.getLocalBounds() }; 
        sprite.setOrigin(sf::Vector2f{bounds.size.x / 2.0f, bounds.size.y / 2.0f}); 
        sprite.setPosition(sf::Vector2f{ position->second.x, position->second.y }); 
        sprite.setScale(sf::Vector2f{chipScale, chipScale});

        target.draw(sprite);
    }
}

void Board::drawDraggedChip(sf::RenderTarget& target) const
{
    if (!drag.active && !drag.animating) return;

    auto it = chipTextures.find(drag.uid);
    if (it == chipTextures.end()) return;

    sf::Sprite sprite{ *it->second };
    sprite.setOrigin({ sprite.getLocalBounds().size.x * 0.5f, sprite.getLocalBounds().size.y * 0.5f });
    sf::Vector2f pos;
    if(drag.active && !drag.animating)
    {
        sf::Color tint = colorset.selected;
        pos = drag.mousePosition - drag.offset;

        if (drag.target != -1)
        {
            if (auto itPt = node.find(drag.target); itPt != node.end())
                pos = { itPt->second.x, itPt->second.y };
        }

        sprite.setColor(tint);
        sprite.setScale({ chipScale, chipScale });
    }
    else if (drag.animating)
    {
        float t = std::clamp(drag.phase, 0.0f, 1.0f);
        float easedT = easeOutBounce(t);

        if (!drag.route.empty())
        {
            size_t segCount = drag.route.size() - 1;
            size_t seg = static_cast<size_t>(easedT * segCount);
            float localT = (easedT * segCount) - seg;

            const auto& a = drag.route[seg];
            const auto& b = drag.route[std::min(seg + 1, segCount)];
            pos = a + (b - a) * localT;

            // Trail
            constexpr size_t trailSteps = 32;
            constexpr float trailSpacing = 0.025f;

            for (size_t i = 0; i < trailSteps; ++i)
            {
                float trailT = easedT - i * trailSpacing;
                if (trailT < 0.0f) break;

                size_t trailSeg = static_cast<size_t>(trailT * segCount);
                float trailLocalT = (trailT * segCount) - trailSeg;

                const auto& ta = drag.route[trailSeg];
                const auto& tb = drag.route[std::min(trailSeg + 1, segCount)];
                sf::Vector2f trailPos = ta + (tb - ta) * trailLocalT;

                sf::Sprite trail{ *it->second };
                trail.setOrigin({ trail.getLocalBounds().size.x * 0.5f, trail.getLocalBounds().size.y * 0.5f });
                trail.setPosition(trailPos);

                float age = static_cast<float>(i) / trailSteps;
                float alpha = 128.0f * ( 1.0f - std::pow(drag.phase, 0.1f));
                float scale = chipScale * (1.0f - age);

                trail.setScale({ scale, scale });
                trail.setColor(sf::Color(255, 255, 255, static_cast<uint8_t>(alpha)));

                target.draw(trail);
            }
        }
        else
        {
            pos = drag.snapback;
            sprite.setColor(colorset.error);
        }

        sprite.setScale({ chipScale, chipScale });
    }
    else
    {
        if (drag.path.empty()) return;
        pos = drag.mousePosition;

        if (drag.target != -1)
        {
            if (auto itPt = node.find(drag.target); itPt != node.end())
                pos = { itPt->second.x, itPt->second.y };

            for (const auto& chip : chips)
                if (chip.position == drag.target && chip.uid != drag.uid)
                    return;
        }

        sprite.setColor(colorset.selected);
        sprite.setScale({ chipScale * 0.8f, chipScale * 0.8f });
    }

    sprite.setPosition(pos);
    target.draw(sprite);
}

void Board::drawHints(sf::RenderTarget& target) const
{
    float centerY = 0.0f;
    for (const auto& [_, point] : node)
    {
        centerY += static_cast<float>(point.y);
    }
    centerY /= static_cast<float>(node.size());

    for (size_t i = 0; i < chips.size(); ++i)
    {
        if (i >= targetPositions.size()) continue;

        const int targetId = targetPositions[i];
        const Chip& chip = chips[i];

        auto itPoint   = node.find(targetId);
        auto itTexture = hintTextures.find(chip.uid);

        if (itPoint == node.end() || itTexture == hintTextures.end()) continue;

        const sf::Vector2f pos
        {
            static_cast<float>(itPoint->second.x),
            static_cast<float>(itPoint->second.y) + ((itPoint->second.y < centerY) ? -64.0f : +64.0f)
        };

        sf::Sprite sprite{ *itTexture->second };
        const sf::FloatRect bounds = sprite.getLocalBounds();

        sprite.setOrigin(sf::Vector2f{ bounds.size.x / 2.0f, bounds.size.y / 2.0f });
        sprite.setPosition(pos);
        sprite.setScale(sf::Vector2f{ hintScale, hintScale });

        if (chip.position == targetId)
        {
            sprite.setColor(colorset.activeHint);
        }
        else
        {
            sprite.setColor(colorset.inactiveHint);
        }

        target.draw(sprite);
    }
}

void Board::mouseDown(const sf::Vector2f& mousePos)
{
    for (auto& button : levelButtons)
    {
        if (button.contains(mousePos))
        {
            button.onClick();
            return;
        }
    }

    drag.active = false;
    constexpr float cellSize = 48.0f;
    constexpr float radius = 24.0f;

    for (const auto& chip : chips)
    {
        auto itPoint = node.find(chip.position);
        auto itTexture = chipTextures.find(chip.uid);
        if (itPoint == node.end() || itTexture == chipTextures.end()) continue;

        sf::Vector2f chipPos{ itPoint->second.x, itPoint->second.y };

        if (distance(mousePos, chipPos) < radius)
        {
            drag.active = true;
            drag.uid = chip.uid;
            drag.origin = chip.position;
            drag.mousePosition = mousePos;
            drag.offset = mousePos - chipPos;
            return;
        }

        sf::Sprite sprite{ *itTexture->second };
        sprite.setOrigin
        (
            sf::Vector2f
            {
                sprite.getLocalBounds().size.x * 0.5f,
                sprite.getLocalBounds().size.y * 0.5f
            }
        );
        sprite.setScale
        (
            sf::Vector2f
            {
                cellSize / static_cast<float>(itTexture->second->getSize().x), 
                cellSize / static_cast<float>(itTexture->second->getSize().x)
            }
        );
        sprite.setPosition(chipPos);

        if (sprite.getGlobalBounds().contains(mousePos))
        {
            drag.active = true;
            drag.uid = chip.uid;
            drag.origin = chip.position;
            drag.mousePosition = mousePos;
            drag.offset = mousePos - chipPos;
            return;
        }
    }

    drag.uid = std::numeric_limits<std::size_t>::max();
}

void Board::mouseMove(const sf::Vector2f& mp)
{
    if (!drag.active) return;

    drag.mousePosition = mp;
    drag.target = -1;

    float minDist = std::numeric_limits<float>::max();

    for (const auto& [id, pt] : node)
    {
        sf::Vector2f ptPos{static_cast<float>(pt.x), static_cast<float>(pt.y)};
        float dist = std::hypot(mp.x - ptPos.x, mp.y - ptPos.y);

        if (dist < minDist)
        {
            minDist = dist;
            drag.target = id;
        }
    }

    if (drag.target != -1)
    {
        drag.path = findPath(drag.origin, drag.target);
    }
    else
    {
        drag.path.clear();
    }
}

void Board::mouseUp()
{
    if (!drag.active) return;

    std::vector<int> path = findPath(drag.origin, drag.target);

    drag.phase = 0.0f;
    drag.active = false;

    Chip* draggedChip = getChipByUid(drag.uid);
    if (!draggedChip) return;

    if (!path.empty())
    {
        drag.animating = true;
        drag.route.clear();

        for (int id : path)
        {
            const Node& pt = node.at(id);
            drag.route.push_back(sf::Vector2f{ pt.x, pt.y });
        }
        draggedChip->position = drag.target;
    }
    else
    {
        draggedChip->position = drag.origin;
        drag.target = -1;
    }

    drag.animating = true;

    const Node& pt = node.at(draggedChip->position);
    drag.snapback = sf::Vector2f{ pt.x, pt.y };
}

void Board::update(float delta)
{
    if(drag.animating)
    {
        drag.phase += delta;
        if (drag.phase > 1.0f)
        {
            drag = DragState{};
        }
        return;
    }
}

void Board::draw(sf::RenderTarget& target, float scale, bool debug, sf::Vector2f mousePos)
{
    update(0.02f);
    for (const PolyLine& line : bakedConnections)
        target.draw(line);

    for (const Honeycomb& honeycomb : bakedHoneycombs)
        target.draw(honeycomb);

    if (drag.active && !drag.path.empty())
    {
        sf::VertexArray pathLine(sf::PrimitiveType::LineStrip, drag.path.size());

        for (size_t i = 0; i < drag.path.size(); ++i)
        {
            auto it = node.find(drag.path[i]);
            if (it == node.end()) continue;

            sf::Vector2f pos(it->second.x, it->second.y);
            pathLine[i].position = pos; 
            pathLine[i].color = colorset.path;
        }

        target.draw(pathLine);
    }

    drawHints(target);        
    drawChips(target);        
    drawDraggedChip(target);  
    for (auto& button : levelButtons)
    {
        button.draw(target);
    }
}

std::vector<int> Board::findPath(int start, int goal) const
{
    std::unordered_map<int, int> cameFrom;
    std::set<int> visited{ start };
    std::vector<int> queue{ start };

    while (!queue.empty())
    {
        int current = queue.front();
        queue.erase(queue.begin());

        if (current == goal) break;

        auto it = adjacency.find(current);
        if (it == adjacency.end()) continue;

        for (int neighbor : it->second)
        {
            if (visited.count(neighbor)) continue;

            bool occupied = std::any_of(chips.begin(), chips.end(),
                [&](const Chip& chip) { return chip.position == neighbor && chip.uid != drag.uid; });

            if (occupied) continue;

            visited.insert(neighbor);
            cameFrom[neighbor] = current;
            queue.push_back(neighbor);
        }
    }

    bool goalOccupied = std::any_of(chips.begin(), chips.end(),
        [&](const Chip& chip) { return chip.position == goal && chip.uid != drag.uid; });

    if (goalOccupied || cameFrom.find(goal) == cameFrom.end())
        return {};

    std::vector<int> path;
    int current = goal;
    while (current != start)
    {
        path.push_back(current);
        current = cameFrom[current];
    }

    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

void Board::clear()
{
    chips.clear();
    node.clear();
    adjacency.clear();
    targetPositions.clear();
    bakedConnections.clear();
    bakedHoneycombs.clear();
    chipTextures.clear();
    hintTextures.clear();

    drag = DragState{};
}

void Board::loadLevel(const std::string& filename, bool external)
{
    clear();
    if(external)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << "\n";
            return;
        }
        loadFromStream(file);
    }
    else 
    {
        std::istringstream stream(filename);
        loadFromStream(stream);
    }

}

void Board::loadFromStream(std::istream& file)
{
    std::string line;

    auto nextLine = [&]() -> bool {
        while (std::getline(file, line)) {
            if (!line.empty()) return true;
        }
        return false;
    };

    int chipCount = 0;
    int pointCount = 0;

    if (nextLine()) chipCount = std::stoi(line);
    if (nextLine()) pointCount = std::stoi(line);

    for (int i = 0; i < pointCount; ++i)
    {
        if (!nextLine()) break;
        auto commaPos = line.find(',');
        if (commaPos == std::string::npos) continue;

        int x = std::stoi(line.substr(0, commaPos));
        int y = std::stoi(line.substr(commaPos + 1));
        addPoint(i + 1, x, y);
    }

    std::vector<int> initialPositions;
    if (nextLine())
    {
        std::istringstream initStream(line);
        std::string token;
        int chipId = 0;

        while (std::getline(initStream, token, ','))
        {
            if (token.empty()) continue;
            int pointId = std::stoi(token);
            initialPositions.push_back(pointId);
            placeChip(++chipId, pointId);
        }
    }

    targetPositions.clear();
    if (nextLine())
    {
        std::istringstream targetStream(line);
        std::string token;

        while (std::getline(targetStream, token, ','))
        {
            if (token.empty()) continue;
            int targetPoint = std::stoi(token);
            targetPositions.push_back(targetPoint);
        }
    }

    int connectionCount = 0;
    if (nextLine()) connectionCount = std::stoi(line);

    for (int i = 0; i < connectionCount; ++i)
    {
        if (!nextLine()) break;
        auto commaPos = line.find(',');
        if (commaPos == std::string::npos) continue;

        int from = std::stoi(line.substr(0, commaPos));
        int to = std::stoi(line.substr(commaPos + 1));
        addConnection(from, to);
    }

    for (auto& [id, pt] : node) pt.type = Node::Intersection;
    for (int targetId : targetPositions)
    {
        if (auto it = node.find(targetId); it != node.end())
            it->second.type = Node::Base;
    }

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (const auto& [_, pt] : node)
    {
        minX = std::min(minX, pt.x);
        maxX = std::max(maxX, pt.x);
        minY = std::min(minY, pt.y);
        maxY = std::max(maxY, pt.y);
    }

    const sf::Vector2f boardCenter = {
        (minX + maxX) / 2.0f,
        (minY + maxY) / 2.0f
    };

    const sf::Vector2f windowCenter = {
        wsize.x / 2.0f,
        wsize.y / 2.0f
    };

    const sf::Vector2f boardOffset = windowCenter - boardCenter;
    for (auto& [_, pt] : node)
    {
        pt.x += boardOffset.x;
        pt.y += boardOffset.y;
    }

    bake();
}

}
