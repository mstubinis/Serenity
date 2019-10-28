#pragma once
#ifndef GAME_MAP_ANCHOR_H
#define GAME_MAP_ANCHOR_H

#include <ecs/Entity.h>

#include <unordered_map>
#include <string>

#include <glm/vec3.hpp>

class Map;
class Anchor final : public EntityWrapper {
    friend class Map;
    private:
        std::unordered_map<std::string, Anchor*> m_Children;
    public:
        Anchor(Map& map, const decimal& x, const decimal& y, const decimal& z);
        Anchor(Map& map, const glm_vec3& position);
        ~Anchor();

        const std::unordered_map<std::string, Anchor*>& getChildren() const;

        const glm_vec3 getPosition();
        void update(const double& dt);
        
        void addChild(const std::string& key, Anchor* anchor);
};
#endif