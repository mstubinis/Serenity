#pragma once
#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <core/engine/scene/Scene.h>
#include <unordered_map>
#include <tuple>
#include "../ships/shipSystems/ShipSystemWeapons.h"
#include "Freelist.h"
#include "../ai/AIIncludes.h"

class Star;
class Ship;
class Planet;
class GameObject;
class EntityWrapper;
class GameSkybox;
class Anchor;
class Client;
class HUD;
class GameplayMode;
class Team;
class Map: public Scene{
    private:
        GameplayMode&                                  m_GameplayMode;
        std::unordered_map<std::string, Planet*>       m_Planets;
        std::unordered_map<std::string, Ship*>         m_Ships;
        std::unordered_map<std::string, Ship*>         m_ShipsPlayerControlled;
        std::unordered_map<std::string, Ship*>         m_ShipsNPCControlled;
        std::string                                    m_Filename;
        std::string                                    m_SkyboxFile;
        Ship*                                          m_Player;
        Client&                                        m_Client;
        HUD*                                           m_HUD;

        Freelist<PrimaryWeaponCannonProjectile*>       m_ActiveCannonProjectiles;
        Freelist<SecondaryWeaponTorpedoProjectile*>    m_ActiveTorpedoProjectiles;

        void loadFromFile(const std::string& file);
    private:
        std::tuple<std::string, Anchor*>               m_RootAnchor;
        std::tuple<std::string, Anchor*>               m_SpawnAnchor;
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const decimal& x = 0, const decimal& y = 0, const decimal& z = 0);
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const glm_vec3& position);
    public:
        Anchor* internalCreateDeepspaceAnchor(const decimal& x, const decimal& y, const decimal& z, const std::string& name = "");
        Anchor* getRootAnchor();
        Anchor* getSpawnAnchor();
        const std::vector<std::string> getClosestAnchor(Anchor* currentAnchor = nullptr);
    public:

        std::vector<EntityWrapper*> m_Objects;

        Map(GameplayMode& mode, Client& client, const std::string& name, const std::string& file);
        virtual ~Map();

        EntityWrapper* getEntityFromName(const std::string& name);
        virtual void update(const double& dt);
        virtual void onResize(const unsigned int& width, const unsigned int& height);
        virtual void render();

        Client& getClient();

        HUD& getHUD();
        Ship* createShip(const AIType::Type ai_type, Team& team, Client& client, const std::string& shipClass, const std::string& shipName, const glm::vec3& position);


        Ship* getPlayer();
        void setPlayer(Ship* p);

        const std::string& skyboxFile() const;

        std::vector<std::string> allowedShips();
        std::string allowedShipsSingleString();

        std::unordered_map<std::string, Planet*>& getPlanets();
        std::unordered_map<std::string, Ship*>& getShips();
        std::unordered_map<std::string, Ship*>& getShipsPlayerControlled();
        std::unordered_map<std::string, Ship*>& getShipsNPCControlled();

        const bool hasShip(const std::string& shipName) const;
        const bool hasShipPlayer(const std::string& shipName) const;
        const bool hasShipNPC(const std::string& shipName) const;

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