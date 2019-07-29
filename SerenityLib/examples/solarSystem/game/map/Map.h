#pragma once
#ifndef GAME_Map_H
#define GAME_Map_H

#include <core/engine/scene/Scene.h>
#include <unordered_map>
#include <tuple>

class Star;
class Ship;
class Planet;
class GameObject;
class EntityWrapper;
class GameSkybox;
class Anchor;
class Map: public Scene{
    private:
        std::unordered_map<std::string, Planet*>   m_Planets;
        std::unordered_map<std::string, Ship*>     m_Ships;
        std::string                                m_Filename;
        std::string                                m_SkyboxFile;
        Ship*                                      m_Player;

        std::tuple<std::string, Anchor*>           m_RootAnchor;
        std::tuple<std::string, Anchor*>           m_SpawnAnchor;

        glm::vec3 m_oldClientPos;
        glm::vec3 m_oldAnchorPos;

        void loadFromFile(const std::string& file);
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const float& x = 0, const float& y = 0, const float& z = 0);
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const glm::vec3& position);
    public:
        Anchor* internalCreateDeepspaceAnchor(const float& x, const float& y, const float& z, const std::string& name = "");

        std::vector<EntityWrapper*> m_Objects;

        Map(const std::string& name, const std::string& file);
        virtual ~Map();

        virtual void update(const double& dt);

        const std::vector<std::string> getClosestAnchor(Anchor* currentAnchor = nullptr);
        

        Ship* getPlayer() { return m_Player; }
        void setPlayer(Ship* p){ m_Player = p; }

        const std::string& skyboxFile() const;

        std::vector<std::string> allowedShips();
        std::string allowedShipsSingleString();

        std::unordered_map<std::string, Planet*>& getPlanets() { return m_Planets; }
        std::unordered_map<std::string, Ship*>& getShips() { return m_Ships; }
        Anchor* getRootAnchor();
        Anchor* getSpawnAnchor();

        /*
        void setAnchor(const float& x, const float& y, const float& z);
        const glm::vec3& getOldClientPos() const;
        const glm::vec3& getOldAnchorPos() const;
        void setOldClientPos(const float& x, const float& y, const float& z);
        void setOldAnchorPos(const float& x, const float& y, const float& z);
        */

};
#endif