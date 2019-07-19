#include "Client.h"
#include "Packet.h"
#include "Core.h"
#include "HUD.h"
#include "gui/Button.h"
#include "gui/Text.h"
#include "gui/specifics/ServerLobbyChatWindow.h"
#include "gui/specifics/ServerLobbyConnectedPlayersWindow.h"

#include "SolarSystem.h"
#include "Ship.h"
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Utils.h>
#include <core/engine/math/Engine_Math.h>

#include <iostream>

using namespace std;
using namespace Engine;

Client::Client(Core& core, sf::TcpSocket* socket) : m_Core(core){
    m_TcpSocket = new Networking::SocketTCP(socket);
    m_username = "";
    m_Validated = false;
    m_InitialConnectionThread = nullptr;
    m_IsCurrentlyConnecting = false;
}
Client::Client(Core& core, const ushort& port, const string& ipAddress) : m_Core(core) {
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
    m_username = "";
    m_Validated = false;
    m_InitialConnectionThread = nullptr;
    m_IsCurrentlyConnecting = false;
}
Client::~Client() {
    SAFE_DELETE_FUTURE(m_InitialConnectionThread);
    SAFE_DELETE(m_TcpSocket);
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
    _client->onReceive();
}
void Client::onReceive() {
    sf::Packet sf_packet;
    const auto& status = receive(sf_packet);

    if (status == sf::Socket::Status::Done) {
        Packet* pp = Packet::getPacket(sf_packet);
        auto& p = *pp;
        if (pp && p.validate(sf_packet)) {
            // Data extracted successfully...
            /*
            if (p.PacketType == PacketType::Server_To_Client_Ship_Physics_Update) {
                if (p.data != "") {
                    SolarSystem& scene = *static_cast<SolarSystem*>(Resources::getCurrentScene());
                    PacketPhysicsUpdate* physics_update = static_cast<PacketPhysicsUpdate*>(pp);
                    auto& phy = *physics_update;
          
                    auto* pbody = scene.getPlayer()->entity().getComponent<ComponentBody>();
                    auto& body = *pbody;
                    btRigidBody& bulletBody = *const_cast<btRigidBody*>(&body.getBody());

                    btTransform centerOfMass;
                    //centerOfMass.setIdentity();

                    const btVector3 pos(phy.px, phy.py, phy.pz);

                    float qx, qy, qz, qw, ax, ay, az, lx, ly, lz;

                    Math::Float32From16(&qx, phy.qx);
                    Math::Float32From16(&qy, phy.qy);
                    Math::Float32From16(&qz, phy.qz);
                    Math::Float32From16(&qw, phy.qw);

                    Math::Float32From16(&ax, phy.ax);
                    Math::Float32From16(&ay, phy.ay);
                    Math::Float32From16(&az, phy.az);

                    const btQuaternion rot(qx, qy, qz, qw);

                    centerOfMass.setOrigin(pos);
                    centerOfMass.setRotation(rot);

                    bulletBody.getMotionState()->setWorldTransform(centerOfMass);
                    bulletBody.setCenterOfMassTransform(centerOfMass);
                    body.clearAllForces();
                    body.setAngularVelocity(ax, ay, az, false);


                    Math::Float32From16(&lx, phy.lx);
                    Math::Float32From16(&ly, phy.ly);
                    Math::Float32From16(&lz, phy.lz);

                    body.setLinearVelocity(lx, ly, lz, false);
                }
            }
            */
            HUD& hud = *m_Core.m_HUD;
            switch (p.PacketType) {
                case PacketType::Server_To_Client_Chat_Message: {
                    PacketChatMessage& pI = *static_cast<PacketChatMessage*>(pp);
                    auto message = pI.name + ": " + pI.data;

                    Text* text = new Text(0, 0, *hud.m_Font, message);
                    text->setColor(1, 1, 0, 1);
                    text->setTextScale(0.62f, 0.62f);
                    hud.m_ServerLobbyChatWindow->addContent(text);
                    break;
                
                }
                case PacketType::Server_To_Client_Client_Joined_Server: {
                    PacketChatMessage& pI = *static_cast<PacketChatMessage*>(pp);
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
                    PacketChatMessage& pI = *static_cast<PacketChatMessage*>(pp);
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