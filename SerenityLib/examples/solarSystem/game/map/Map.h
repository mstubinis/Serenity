#pragma once
#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <core/engine/scene/Scene.h>
#include <unordered_map>
#include <tuple>
#include "../ships/shipSystems/ShipSystemWeapons.h"
#include "Freelist.h"

class Star;
class Ship;
class Planet;
class GameObject;
class EntityWrapper;
class GameSkybox;
class Anchor;
class Client;
class HUD;
class Map: public Scene{
    private:
        std::unordered_map<std::string, Planet*>       m_Planets;
        std::unordered_map<std::string, Ship*>         m_Ships;
        std::string                                    m_Filename;
        std::string                                    m_SkyboxFile;
        Ship*                                          m_Player;
        Client&                                        m_Client;
        HUD*                                           m_HUD;

        std::tuple<std::string, Anchor*>               m_RootAnchor;
        std::tuple<std::string, Anchor*>               m_SpawnAnchor;

        Freelist<PrimaryWeaponCannonProjectile*>       m_ActiveCannonProjectiles;
        Freelist<SecondaryWeaponTorpedoProjectile*>    m_ActiveTorpedoProjectiles;

        void loadFromFile(const std::string& file);
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const float& x = 0, const float& y = 0, const float& z = 0);
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const glm::vec3& position);
    public:
        Anchor* internalCreateDeepspaceAnchor(const float& x, const float& y, const float& z, const std::string& name = "");

        std::vector<EntityWrapper*> m_Objects;

        Map(Client&, const std::string& name, const std::string& file);
        virtual ~Map();

        virtual void update(const double& dt);
        virtual void onResize(const unsigned int& width, const unsigned int& height);
        virtual void render();

        Client& getClient();

        const std::vector<std::string> getClosestAnchor(Anchor* currentAnchor = nullptr);
        const bool hasShip(const std::string& shipName) const;
        
        HUD& getHUD();
        Ship* createShip(Client& client, const std::string& shipClass, const std::string& shipName, const bool& playerShip, const glm::vec3& position);


        Ship* getPlayer();
        void setPlayer(Ship* p);

        const std::string& skyboxFile() const;

        std::vector<std::string> allowedShips();
        std::string allowedShipsSingleString();

        std::unordered_map<std::string, Planet*>& getPlanets() { return m_Planets; }
        std::unordered_map<std::string, Ship*>& getShips() { return m_Ships; }
        Anchor* getRootAnchor();
        Anchor* getSpawnAnchor();

        const int addCannonProjectile(PrimaryWeaponCannonProjectile*, const int index = -1);
        const int addTorpedoProjectile(SecondaryWeaponTorpedoProjectile*, const int index = -1);

        PrimaryWeaponCannonProjectile* getCannonProjectile(const int index);
        SecondaryWeaponTorpedoProjectile* getTorpedoProjectile(const int index);

        void removeCannonProjectile(const int index);
        void removeTorpedoProjectile(const int index);

        const int try_addCannonProjectile();
        const int try_addTorpedoProjectile();
        const bool try_addCannonProjectile(const int requestedIndex);
        const bool try_addTorpedoProjectile(const int requestedIndex);
};
#endif