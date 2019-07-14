#include "Client.h"
#include "Packet.h"

#include "SolarSystem.h"
#include "Ship.h"
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine_Utils.h>
#include <core/engine/Engine_Math.h>

#include <iostream>

using namespace std;
using namespace Engine;

Client::Client(sf::TcpSocket* socket) {
    m_client = new Networking::SocketTCP(socket);
    m_client->setBlocking(false);
}
Client::Client(const ushort& port, const string& ipAddress) {
    m_client = new Networking::SocketTCP(port, ipAddress);
}
Client::~Client() {
    SAFE_DELETE(m_client);
}
void Client::connect(const ushort& timeout) {
    if (m_client && !m_client->connected()) {
        const auto& status = m_client->connect(timeout);
        if (status == sf::Socket::Status::Done) {
            std::cout << "Client Connected!" << std::endl;
            m_client->setBlocking(false);
        }
    }
}
void Client::changeConnectionDestination(const ushort& port, const string& ipAddress) {
    disconnect();
    SAFE_DELETE(m_client);
    m_client = new Networking::SocketTCP(port, ipAddress);
}
void Client::connect() { connect(0); }
void Client::disconnect() {
    if (m_client) {
        m_client->disconnect();
    }
}
const bool Client::connected() const {
    if (!m_client) 
        return false;
    return m_client->connected();
}
const sf::Socket::Status Client::send(Packet& packet) {
    if (!m_client) return sf::Socket::Status::Error;
    sf::Packet sf_packet;
    packet.build(sf_packet);
    return m_client->send(sf_packet);
}
const sf::Socket::Status Client::send(sf::Packet& packet) {
    if (!m_client) return sf::Socket::Status::Error;
    return m_client->send(packet);
}
const sf::Socket::Status Client::send(const void* data, size_t size) {
    if (!m_client) return sf::Socket::Status::Error;
    return m_client->send(data, size);
}
const sf::Socket::Status Client::send(const void* data, size_t size, size_t& sent) {
    if (!m_client) return sf::Socket::Status::Error;
    return m_client->send(data, size, sent);
}
const sf::Socket::Status Client::receive(sf::Packet& packet) {
    if (!m_client) return sf::Socket::Status::Error;
    return m_client->receive(packet);
}
const sf::Socket::Status Client::receive(void* data, size_t size, size_t& received) {
    if (!m_client) return sf::Socket::Status::Error;
    return m_client->receive(data,size,received);
}


void epriv::ClientInternalPublicInterface::update(Client* _client) {
    if (!_client) return;
    auto& client = *_client;
    //while (client.connected()) {
        client.onReceive();
    //}
}
void Client::onReceive() {
    sf::Packet sf_packet;
    const auto& status = receive(sf_packet);
    if (status == sf::Socket::Status::Done) {
        Packet* pp = Packet::getPacket(sf_packet);
        auto& p = *pp;
        if (pp && p.validate(sf_packet)) {
            // Data extracted successfully...
            std::cout << "Client: ";
            p.print();

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
        }
        SAFE_DELETE(pp);
    }
}