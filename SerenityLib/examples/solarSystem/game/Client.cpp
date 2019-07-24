#include "Client.h"
#include "Packet.h"
#include "Core.h"
#include "HUD.h"
#include "Helper.h"
#include "ResourceManifest.h"
#include "gui/Button.h"
#include "gui/Text.h"
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"
#include "gui/specifics/ServerLobbyShipSelectorWindow.h"
#include "gui/specifics/ServerHostingMapSelectorWindow.h"

#include "SolarSystem.h"
#include "GameSkybox.h"
#include "Ship.h"
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Utils.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Camera.h>

#include <iostream>

using namespace std;
using namespace Engine;

struct ShipSelectorButtonOnClick final {void operator()(Button* button) const {
    ServerLobbyShipSelectorWindow& window = *static_cast<ServerLobbyShipSelectorWindow*>(button->getUserPointer());
    for (auto& widget : window.getWindowFrame().content()) {
        widget->setColor(0.5f, 0.5f, 0.5f, 0.0f);
    }
    button->setColor(0.5f, 0.5f, 0.5f, 1.0f);

    auto& shipname = button->text();
    auto& handles = ResourceManifest::Ships.at(shipname);
    window.m_ChosenShipName = shipname;
    EntityWrapper& entity = *static_cast<EntityWrapper*>(window.getUserPointer());
    ComponentModel& model = *entity.entity().getComponent<ComponentModel>();
    model.setModelMesh(handles.get<0>(),0);
    model.setModelMaterial(handles.get<1>(), 0);
    model.show();

    auto& camera = const_cast<Camera&>(window.getShipDisplay().getCamera());
    camera.entity().getComponent<ComponentLogic2>()->call(-0.0001);
}};

Client::Client(Core& core, sf::TcpSocket* socket) : m_Core(core){
    m_TcpSocket = new Networking::SocketTCP(socket);
    internalInit();
}
Client::Client(Core& core, const ushort& port, const string& ipAddress) : m_Core(core) {
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
    internalInit();
}

Client::~Client() {
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
}
void Client::internalInit() {
    m_username = "";
    m_mapname = "";
    m_Validated = false;
    m_PingTime = 0.0;
    m_Timeout = 0.0;
    m_InitialConnectionThread = nullptr;
    m_IsCurrentlyConnecting = false;
}
void Client::changeConnectionDestination(const ushort& port, const string& ipAddress) {
    m_IsCurrentlyConnecting = false;
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
}
const sf::Socket::Status Client::connect(const ushort& timeout) {
    if (m_TcpSocket->isBlocking()) {
        auto conn = [&](Client* client, const ushort timeout) {
            client->m_IsCurrentlyConnecting = true;
            m_Core.m_HUD->setNormalText("Connecting...", static_cast<float>(timeout) + 2.2f);
            const auto status = client->m_TcpSocket->connect(timeout);
            if (status == sf::Socket::Status::Done) {
                m_Core.m_HUD->setGoodText("Connected!", 2);
                client->m_TcpSocket->setBlocking(false);
                m_Core.requestValidation(m_username);
            }else if (status == sf::Socket::Status::Error) {
                m_Core.m_HUD->setErrorText("Connection to the server failed",20);
            }else if (status == sf::Socket::Status::Disconnected) {
                m_Core.m_HUD->setErrorText("Disconnected from the server",20);
            }
            client->m_IsCurrentlyConnecting = false;
            return status;
        };
        //return conn(this, timeout);
        SAFE_DELETE_FUTURE(m_InitialConnectionThread);
        m_InitialConnectionThread = new std::future<sf::Socket::Status>(std::move(std::async(std::launch::async, conn, this, timeout)));
    }else{
        const auto status = m_TcpSocket->connect(timeout);
        if (status == sf::Socket::Status::Done) {
            m_Core.requestValidation(m_username);
        }
        return status;
    }
    return sf::Socket::Status::Error;
}
void Client::disconnect() {
    m_TcpSocket->disconnect();
}
const sf::Socket::Status Client::send(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    const auto status = m_TcpSocket->send(sf_packet);
    return status;
}
const sf::Socket::Status Client::send(sf::Packet& packet) {
    const auto status = m_TcpSocket->send(packet);
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size) {
    const auto status = m_TcpSocket->send(data, size);
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size, size_t& sent) {
    const auto status = m_TcpSocket->send(data, size, sent);
    return status;
}
const sf::Socket::Status Client::receive(sf::Packet& packet) {
    const auto status = m_TcpSocket->receive(packet);
    return status;
}
const sf::Socket::Status Client::receive(void* data, size_t size, size_t& received) {
    const auto status = m_TcpSocket->receive(data,size,received);
    return status;
}
const string& Client::username() const {
    return m_username;
}

void epriv::ClientInternalPublicInterface::update(Client* _client) {
    if (!_client) 
        return;
    auto& client = *_client;
    const auto& dt = Resources::dt();
    client.m_Timeout += dt;
    if (client.m_Core.gameState() == GameState::Game) {
        client.m_PingTime += dt;
        if (client.m_PingTime > 0.2) {
            //keep pinging the server, sending your ship physics info
            auto& map = *static_cast<SolarSystem*>(Resources::getCurrentScene());
            auto& playerShip = *map.getPlayer();
            PacketPhysicsUpdate p(playerShip, map);
            p.PacketType = PacketType::Client_To_Server_Ship_Physics_Update;
            if (!p.data.empty()) {
                p.data = client.m_username + "," + p.data;
            }else{
                p.data = client.m_username;
            }
            client.send(p);
            client.m_PingTime = 0.0;
        }
    }
    client.onReceive();
}
void Client::onReceive() {
    sf::Packet sf_packet;
    const auto& status = receive(sf_packet);

    if (status == sf::Socket::Status::Done) {
        Packet* pp = Packet::getPacket(sf_packet);
        auto& p = *pp;
        if (pp && p.validate(sf_packet)) {
            // Data extracted successfully...
            m_Timeout = 0.0;
            HUD& hud = *m_Core.m_HUD;
            switch (p.PacketType) {
                case PacketType::Server_To_Client_Ship_Physics_Update: {
                    if (m_Core.gameState() == GameState::Game) { //TODO: figure out a way for the server to only send phyiscs updates to clients in the map
                        PacketPhysicsUpdate& pI = *static_cast<PacketPhysicsUpdate*>(pp);

                        auto& map = *static_cast<SolarSystem*>(Resources::getCurrentScene());

                        auto info = Helper::SeparateStringByCharacter(pI.data, ',');
                        auto& playername = info[0];
                        auto& shipclass = info[1];

                        const auto& offset = map.getAnchor();
                        const float& x = pI.px + offset.x;
                        const float& y = pI.py + offset.y;
                        const float& z = pI.pz + offset.z;

                        Ship* ship = nullptr;
                        if (!map.getShips().count(playername)) {
                            auto handles = ResourceManifest::Ships[shipclass];
                            ship = new Ship(handles.get<0>(), handles.get<1>(), shipclass, false, playername, glm::vec3(x, y, z), glm::vec3(1.0f), CollisionType::ConvexHull, &map);
                            
                        }else{
                            ship = map.getShips().at(playername);
                        }
                        auto& body = *ship->entity().getComponent<ComponentBody>();
                        btRigidBody& bulletBody = *const_cast<btRigidBody*>(&body.getBody());

                        btTransform centerOfMass;
                        const btVector3 pos(x, y, z);

                        float qx, qy, qz, qw, ax, ay, az, lx, ly, lz;

                        Math::Float32From16(&qx, pI.qx);  Math::Float32From16(&qy, pI.qy);  Math::Float32From16(&qz, pI.qz);  Math::Float32From16(&qw, pI.qw);

                        Math::Float32From16(&ax, pI.ax);  Math::Float32From16(&ay, pI.ay);  Math::Float32From16(&az, pI.az);

                        const btQuaternion rot(qx, qy, qz, qw);

                        centerOfMass.setOrigin(pos);  centerOfMass.setRotation(rot);

                        bulletBody.getMotionState()->setWorldTransform(centerOfMass);
                        bulletBody.setCenterOfMassTransform(centerOfMass);
                        body.clearAllForces();
                        body.setAngularVelocity(ax, ay, az, false);

                        Math::Float32From16(&lx, pI.lx);  Math::Float32From16(&ly, pI.ly);  Math::Float32From16(&lz, pI.lz);

                        body.setLinearVelocity(lx, ly, lz, false);
                    }
                    break;
                }
                case PacketType::Server_To_Client_New_Client_Entered_Map: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    
                    auto info = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map

                    SolarSystem& map = *static_cast<SolarSystem*>(Resources::getScene(info[1]));
                    auto& handles = ResourceManifest::Ships.at(info[0]);
                    Ship* ship = new Ship(handles.get<0>(), handles.get<1>(), info[0], false, pI.name, glm::vec3(pI.r, pI.g, pI.b), glm::vec3(1.0f), CollisionType::ConvexHull, &map);

                    break;
                }
                case PacketType::Server_To_Client_Approve_Map_Entry: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);

                    auto info = Helper::SeparateStringByCharacter(pI.data, ','); //shipclass,map

                    hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(false);
                    m_Core.enterMap(info[1], info[0], pI.name, pI.r, pI.g, pI.b);
                    hud.m_Next->setText("Next");
                    hud.m_GameState = GameState::Game;
                    break;
                }
                case PacketType::Server_To_Client_Reject_Map_Entry: {
                    break;
                }
                case PacketType::Server_To_Client_Map_Data: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto& mapname = pI.name;
                    m_mapname = mapname;
                    SolarSystem* map = static_cast<SolarSystem*>(Resources::getScene(mapname));
                    if (!map) {
                        map = new SolarSystem(mapname, ResourceManifest::BasePath + "data/Systems/" + mapname + ".txt");
                    }
                    auto& menuScene = *const_cast<Scene*>(Resources::getScene("Menu"));
                    auto* menuSkybox = menuScene.skybox();
                    SAFE_DELETE(menuSkybox);
                    GameSkybox* newMenuSkybox = new GameSkybox(map->skyboxFile(), 0);
                    menuScene.setSkybox(newMenuSkybox);
                    menuScene.setGlobalIllumination(map->getGlobalIllumination());

                    hud.m_ServerLobbyShipSelectorWindow->clear();
                    auto ships = map->allowedShips();
                    for (auto& ship : ships) {
                        Button* shipbutton = new Button(*hud.m_Font, 0, 0, 100, 40);
                        shipbutton->setText(ship);
                        shipbutton->setColor(0.5f, 0.5f, 0.5f, 0.0f);
                        shipbutton->setTextColor(1, 1, 1, 1);
                        shipbutton->setAlignment(Alignment::TopLeft);
                        shipbutton->setWidth(600);
                        shipbutton->setTextAlignment(TextAlignment::Left);
                        shipbutton->setUserPointer(hud.m_ServerLobbyShipSelectorWindow);
                        shipbutton->setOnClickFunctor(ShipSelectorButtonOnClick());
                        hud.m_ServerLobbyShipSelectorWindow->addContent(shipbutton);
                    }
                    hud.m_ServerLobbyShipSelectorWindow->setShipViewportActive(true);
                    break;
                }
                case PacketType::Server_To_Client_Chat_Message: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto message = pI.name + ": " + pI.data;

                    Text* text = new Text(0, 0, *hud.m_Font, message);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text);
                    break;
                
                }
                case PacketType::Server_To_Client_Client_Joined_Server: {
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto message = pI.name + ": Has joined the server";


                    Text* text = new Text(0, 0, *hud.m_Font, pI.name);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyConnectedPlayersWindow->addContent(text);

                    Text* text1 = new Text(0, 0, *hud.m_Font, message);
                    text1->setColor(0.8f, 1, 0.2f, 1);
                    text1->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text1);
                    break;
                }
                case PacketType::Server_To_Client_Client_Left_Server:{
                    PacketMessage& pI = *static_cast<PacketMessage*>(pp);
                    auto message = pI.name + ": Has left the server";

                    Text* text = new Text(0, 0, *hud.m_Font, pI.name);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyConnectedPlayersWindow->removeContent(pI.name);

                    Text* text1 = new Text(0, 0, *hud.m_Font, message);
                    text1->setColor(0.907f, 0.341f, 0.341f, 1.0f);
                    text1->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text1);
                    break;
                }
                case PacketType::Server_To_Client_Accept_Connection: {
                    m_Validated = true;
                    if (m_Core.m_GameState != GameState::Host_Server_Lobby_And_Ship && m_Core.m_GameState == GameState::Host_Server_Port_And_Name_And_Map) {
                        m_Core.m_GameState = GameState::Host_Server_Lobby_And_Ship;
                        hud.m_Next->setText("Enter Game");
                    }else if (m_Core.m_GameState != GameState::Join_Server_Server_Lobby && m_Core.m_GameState == GameState::Join_Server_Port_And_Name_And_IP) {
                        m_Core.m_GameState = GameState::Join_Server_Server_Lobby;
                        hud.m_Next->setText("Enter Game");
                    }
                    hud.m_ServerLobbyConnectedPlayersWindow->clear();
                    hud.m_ServerLobbyChatWindow->clear();
                    stringstream ss(pp->data);
                    vector<string> result;
                    while (ss.good()){
                        string substr;
                        getline(ss, substr, ',');
                        result.push_back(substr);
                    }
                    if (result.size() == 0 && !pp->data.empty()) {
                        result.push_back(pp->data);
                    }
                    //result is a vector of connected players
                    for (auto& _name : result) {
                        if (!_name.empty()) {
                            Text* text = new Text(0, 0, *hud.m_Font, _name);
                            text->setColor(1, 1, 0, 1);
                            text->setTextScale(0.62f, 0.62f);
                            hud.m_ServerLobbyConnectedPlayersWindow->addContent(text);
                        }
                    }
                    break;
                }case PacketType::Server_To_Client_Reject_Connection: {
                    m_Validated = false;
                    hud.setErrorText("Someone has already chosen that name");
                    break;
                }case PacketType::Server_Shutdown: {
                    m_Validated = false;
                    m_Core.shutdownClient(true);
                    hud.setErrorText("Disconnected from the server",600);
                    m_Core.m_GameState = GameState::Main_Menu;
                    hud.m_Next->setText("Next");

                    hud.m_ServerLobbyChatWindow->clear();
                    hud.m_ServerLobbyConnectedPlayersWindow->clear();
                    break;
                }default: {
                    break;
                }
            }
        }
        SAFE_DELETE(pp);
    }
}