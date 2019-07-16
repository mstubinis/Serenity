#include "Client.h"
#include "Packet.h"
#include "Core.h"
#include "HUD.h"
#include "gui/Button.h"

#include "SolarSystem.h"
#include "Ship.h"
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Utils.h>
#include <core/engine/Engine_Math.h>

#include <iostream>

using namespace std;
using namespace Engine;

Client::Client(Core& core, sf::TcpSocket* socket) : m_Core(core){
    m_TcpSocket = new Networking::SocketTCP(socket);
    m_TcpSocket->setBlocking(false);
    m_username = "";
    m_Validated = false;
    m_Connected = false;
}
Client::Client(Core& core, const ushort& port, const string& ipAddress) : m_Core(core) {
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
    m_TcpSocket->setBlocking(false);
    m_username = "";
    m_Validated = false;
    m_Connected = false;
}
Client::~Client() {
    SAFE_DELETE(m_TcpSocket);
    m_Connected = false;
}
void Client::changeConnectionDestination(const ushort& port, const string& ipAddress) {
    disconnect();
    SAFE_DELETE(m_TcpSocket);
    m_TcpSocket = new Networking::SocketTCP(port, ipAddress);
    m_TcpSocket->setBlocking(false);
}
bool Client::connect(const ushort& timeout) {
    const auto status = m_TcpSocket->connect(timeout);
    if (status != sf::Socket::Status::Error && status != sf::Socket::Status::Disconnected) {
        m_Connected = true;
    }else{
        m_Connected = false;
    }
    return m_Connected;
}
void Client::disconnect() {
    m_TcpSocket->disconnect();
    m_Connected = false;
}
const bool Client::connected() const {
    return m_Connected;
}
const sf::Socket::Status Client::send(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    const auto status = m_TcpSocket->send(sf_packet);
    if (status == sf::Socket::Status::Error || status == sf::Socket::Status::Disconnected) {
        m_Connected = false;
    }
    return status;
}
const sf::Socket::Status Client::send(sf::Packet& packet) {
    const auto status = m_TcpSocket->send(packet);
    if (status == sf::Socket::Status::Error || status == sf::Socket::Status::Disconnected) {
        m_Connected = false;
    }
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size) {
    const auto status = m_TcpSocket->send(data, size);
    if (status == sf::Socket::Status::Error || status == sf::Socket::Status::Disconnected) {
        m_Connected = false;
    }
    return status;
}
const sf::Socket::Status Client::send(const void* data, size_t size, size_t& sent) {
    const auto status = m_TcpSocket->send(data, size, sent);
    if (status == sf::Socket::Status::Error || status == sf::Socket::Status::Disconnected) {
        m_Connected = false;
    }
    return status;
}
const sf::Socket::Status Client::receive(sf::Packet& packet) {
    const auto status = m_TcpSocket->receive(packet);
    if (status == sf::Socket::Status::Error || status == sf::Socket::Status::Disconnected) {
        m_Connected = false;
    }
    return status;
}
const sf::Socket::Status Client::receive(void* data, size_t size, size_t& received) {
    const auto status = m_TcpSocket->receive(data,size,received);
    if (status == sf::Socket::Status::Error || status == sf::Socket::Status::Disconnected) {
        m_Connected = false;
    }
    return status;
}


void epriv::ClientInternalPublicInterface::update(Client* _client) {
    if (!_client) 
        return;
    _client->onReceive();
}
void Client::onReceive() {
    sf::Packet sf_packet;
    const auto& status = receive(sf_packet);

    if (status == sf::Socket::Disconnected) {
        m_TcpSocket->connect();
        return;
    }

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

            switch (p.PacketType) {
                case PacketType::Server_To_Client_Accept_Connection: {
                    std::cout << "Client: Server_To_Client_Accept_Connection: Received" << std::endl;
                    m_Validated = true;
                    if (m_Core.m_GameState != GameState::Host_Server_Lobby_And_Ship && m_Core.m_GameState == GameState::Host_Server_Port_And_Name_And_Map) {
                        m_Core.m_GameState = GameState::Host_Server_Lobby_And_Ship;
                        m_Core.m_HUD->m_Next->setText("Enter Game");
                    }
                    else if (m_Core.m_GameState != GameState::Join_Server_Server_Lobby && m_Core.m_GameState == GameState::Join_Server_Port_And_Name_And_IP) {
                        m_Core.m_GameState = GameState::Join_Server_Server_Lobby;
                        m_Core.m_HUD->m_Next->setText("Enter Game");
                    }
                    break;
                }
                case PacketType::Server_To_Client_Reject_Connection: {
                    m_Validated = false;
                    std::cout << "Client: Server_To_Client_Reject_Connection: Received" << std::endl;
                    m_Core.m_HUD->setErrorText("Someone has already chosen that name");
                    break;
                }
                case PacketType::Server_Shutdown: {
                    m_Validated = false;
                    std::cout << "Client: Server_Shutdown: Received" << std::endl;
                    m_Core.shutdownClient();
                    m_Core.m_HUD->setErrorText("Disconnected from server",600);
                    m_Core.m_GameState = GameState::Main_Menu;
                    m_Core.m_HUD->m_Next->setText("Next");
                    break;
                }
                default: {
                    break;
                }
            }
        }
        SAFE_DELETE(pp);
    }
}