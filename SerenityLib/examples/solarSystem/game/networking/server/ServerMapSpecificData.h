#pragma once
#ifndef GAME_SERVER_MAP_SPECIFIC_DATA_H
#define GAME_SERVER_MAP_SPECIFIC_DATA_H

#include <unordered_map>
#include <string>

class  Server;
class  Map;
struct PacketCollisionEvent;
struct Packet;
//a simple data structure to coordinate ship respawning
class ShipRespawning final {
    private:
        std::unordered_map<std::string, std::tuple<std::string, std::string, double>> m_Ships; //key = shipkey, value = ship class,  closest spawn anchor, respawn time left
        Server& m_Server;
    public:
        ShipRespawning(Server&);
        ~ShipRespawning();

        void processShip(const std::string& shipMapKey, const std::string& shipClass, const std::string& closest_spawn_anchor);

        void cleanup();
        void update(const double& dt);
};
//a simple data structure to coordinate ship on ship / station / whatever collisions
class CollisionEntries final {
    private:
        std::unordered_map<std::string, double> m_CollisionPairs; //key = ship1.key + "|" + ship2.key, double is time left until another collision entry can be processed
        Server& m_Server;

        void internal_send_packet(const PacketCollisionEvent&);

    public:
        CollisionEntries(Server&);
        ~CollisionEntries();

        void processCollision(const PacketCollisionEvent& packet, Map& map);

        void cleanup();
        void update(const double& dt);
};


class ServerMapSpecificData final {
    friend class Server;
    private:
        Server&                                        m_Server;
        Map*                                           m_Map;
        CollisionEntries                               m_CollisionEntries;
        ShipRespawning                                 m_RespawningShips;
        double                                         m_DeepspaceAnchorTimer;
        double                                         m_PhysicsUpdateTimer;

        void internal_process_deepspace_anchor(Packet*);

    public:
        ServerMapSpecificData(Server&);
        ~ServerMapSpecificData();

        void cleanup();

        void update(const double& dt);
};

#endif