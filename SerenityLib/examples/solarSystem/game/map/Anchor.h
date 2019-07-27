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
        //glm::vec3                                m_Midpoint;
    public:
        Anchor(Map& map, const float& x, const float& y, const float& z);
        Anchor(Map& map, const glm::vec3& position);
        ~Anchor();

        const std::unordered_map<std::string, Anchor*>& getChildren() const;

        void finalize_this_only();
        void finalize_all();

        //const glm::vec3& getMidpoint() const;
        const glm::vec3& getPosition();
        void update(const double& dt);
        

        void addChild(const std::string& key, Anchor* anchor);

};


#endif