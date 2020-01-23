#pragma once
#ifndef GAME_MAP_ANCHOR_H
#define GAME_MAP_ANCHOR_H

#include <ecs/EntityWrapper.h>

#include <unordered_map>
#include <string>

#include <glm/vec3.hpp>

class Map;
class Anchor final : public EntityWrapper {
    friend class Map;
    private:
        std::vector<Anchor*>  m_Children;
        std::string           m_Name;
    public:
        Anchor(const std::string& name, Map& map, const decimal& x, const decimal& y, const decimal& z);
        Anchor(const std::string& name, Map& map, const glm_vec3& position);
        ~Anchor();

        Anchor(const Anchor& other)                = delete;
        Anchor& operator=(const Anchor& other)     = delete;
        Anchor(Anchor&& other) noexcept            = delete;
        Anchor& operator=(Anchor&& other) noexcept = delete;

        const std::vector<Anchor*>& getChildren() const;

        void setName(const std::string&);
        const std::string& getName() const;
        const glm_vec3 getPosition();
        void update(const double& dt);
        
        Anchor* getChild(const std::string& childName);
        void addChild(Anchor* anchor);
};
#endif