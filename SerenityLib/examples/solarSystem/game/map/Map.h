#pragma once
#ifndef GAME_MAP_H
#define GAME_MAP_H

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
class Server;
class LoadingScreen;

#include <core/engine/scene/Scene.h>
#include <unordered_map>
#include <tuple>
#include "../ships/shipSystems/ShipSystemWeapons.h"
#include "Freelist.h"
#include "../ai/AIIncludes.h"

class Map: public Scene{
    friend class Server;

    struct MapLoadStatus final { enum Status {
        CompletelyEmpty,
        PartiallyLoaded,
        FullyLoaded,
    };};
 
    private:
        Map::MapLoadStatus::Status                           m_LoadStatus;
        GameplayMode&                                        m_GameplayMode;
        std::unordered_map<std::string, Planet*>             m_Planets;
        std::unordered_map<std::string, Ship*>               m_Ships;
        std::unordered_map<std::string, Ship*>               m_ShipsPlayerControlled;
        std::unordered_map<std::string, Ship*>               m_ShipsNPCControlled;
        std::string                                          m_Filename;
        std::string                                          m_SkyboxFile;
        Ship*                                                m_Player;
        Client&                                              m_Client;
        HUD*                                                 m_HUD;
        bool                                                 m_IsServer;
        unsigned int                                         m_TotalItemsCountForLoading;

        Freelist<PrimaryWeaponCannonProjectile*>             m_ActiveCannonProjectiles;
        Freelist<SecondaryWeaponTorpedoProjectile*>          m_ActiveTorpedoProjectiles;

        //[0] is the root anchor, rest are spawn anchors
        std::vector<Anchor*>                                 m_SpawnAnchors;

        //give the map its essentials, but do not load everything needed
        void basic_init(const std::string& file);

        void loading_screen_render(LoadingScreen* loadingScreen, const float& progress);
 
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const decimal& x = 0, const decimal& y = 0, const decimal& z = 0);
        Anchor* internalCreateAnchor(const std::string& parentAnchor, const std::string& thisName, std::unordered_map<std::string, Anchor*>& loadedAnchors, const glm_vec3& position);
    public:
        Anchor* internalCreateDeepspaceAnchor(const decimal& x, const decimal& y, const decimal& z, const std::string& name = "");
        Anchor*                        getRootAnchor();
        Anchor*                        getSpawnAnchor(const std::string& name = "");
        const std::vector<std::string> getClosestAnchor(Anchor* currentAnchor = nullptr, Ship* ship = nullptr);
        const std::string              getClosestSpawnAnchor(Ship* ship = nullptr);
    public:
        std::vector<EntityWrapper*> m_Objects;

        Map(GameplayMode& mode, Client& client, const std::string& name, const std::string& file);
        virtual ~Map();

        const bool full_load(LoadingScreen* loadingScreen = nullptr);
        const bool isFullyLoaded() const;

        const bool& isServer() const;
        void cleanup();

        EntityWrapper* getEntityFromName(const std::string& name);
        virtual void update(const double& dt);
        virtual void onResize(const unsigned int& width, const unsigned int& height);
        virtual void render();


        void clear_source_of_all_threat(const std::string& source);

        Client& getClient();

        HUD& getHUD();

        static Ship* createShipDull(const std::string& shipClass, const glm::vec3& position, Scene* scene);
        Ship* createShip(AIType::Type ai_type, Team& team, Client& client, const std::string& shipClass, const std::string& shipName, const glm::vec3& position);


        Ship* getPlayer();
        void setPlayer(Ship* p);

        const std::string& getFilename() const;
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

        const int  get_and_use_next_cannon_projectile_index();
        const bool try_addCannonProjectile(const int requestedIndex);

        const int  get_and_use_next_torpedo_projectile_index();
        const bool try_addTorpedoProjectile(const int requestedIndex);
};
#endif