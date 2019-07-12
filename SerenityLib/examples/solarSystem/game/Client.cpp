#include "Client.h"
#include "Packet.h"

#include <core/engine/Engine_Utils.h>

#include <core/engine/Engine_Math.h>

#include <iostream>

using namespace std;
using namespace Engine;

Client::Client(sf::TcpSocket* socket) {
    m_client = new Networking::SocketTCP(socket);
    m_client->setBlocking(false);
}
Client::Client(const ushort& port, const std::string& ipAddress) {
    m_client = new Networking::SocketTCP(port, ipAddress);
}
Client::~Client() {
    SAFE_DELETE(m_client);
}
void Client::connect(const ushort& timeout) {
    if (!m_client->connected()) {
        const auto& status = m_client->connect(timeout);
        if (status == sf::Socket::Status::Done) {
            std::cout << "Client Connected!" << std::endl;
            m_client->setBlocking(false);
        }else{
            //error
        }
    }
}
void Client::connect() {
    connect(0);
}
void Client::disconnect() {
    m_client->disconnect();
    SAFE_DELETE(m_client);
}

const bool Client::connected() const {
    return m_client->connected();
}

const sf::Socket::Status Client::send(Packet& packet) {
    sf::Packet sf_packet;

    sf_packet <<
        packet.PacketType <<
        packet.x <<
        packet.y <<
        packet.z <<
        packet.x1 <<
        packet.y1 <<
        packet.z1 <<
        packet.x2 <<
        packet.y2 <<
        packet.z2 <<
        packet.x3 <<
        packet.y3 <<
        packet.z3;

    return m_client->send(sf_packet);
}

const sf::Socket::Status Client::send(sf::Packet& packet) {
    return m_client->send(packet);
}
const sf::Socket::Status Client::send(const void* data, size_t size) {
    return m_client->send(data, size);
}
const sf::Socket::Status Client::send(const void* data, size_t size, size_t& sent) {
    return m_client->send(data, size, sent);
}
const sf::Socket::Status Client::receive(sf::Packet& packet) {
    return m_client->receive(packet);
}
const sf::Socket::Status Client::receive(void* data, size_t size, size_t& received) {
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

    }
    Packet p;

    if (sf_packet >> p.PacketType >> p.x >> p.y >> p.z >> p.x1 >> p.y1 >> p.z1 >> p.x2 >> p.y2 >> p.z2 >> p.x3 >> p.y3 >> p.z3) {
        // Data extracted successfully...

        float x, y, z, x1, y1, z1, x2, y2, z2, x3, y3, z3;
        Engine::Math::Float32From16(&x, p.x);
        Engine::Math::Float32From16(&y, p.y);
        Engine::Math::Float32From16(&z, p.z);

        Engine::Math::Float32From16(&x1, p.x1);
        Engine::Math::Float32From16(&y1, p.y1);
        Engine::Math::Float32From16(&z1, p.z1);

        Engine::Math::Float32From16(&x2, p.x2);
        Engine::Math::Float32From16(&y2, p.y2);
        Engine::Math::Float32From16(&z2, p.z2);

        Engine::Math::Float32From16(&x3, p.x3);
        Engine::Math::Float32From16(&y3, p.y3);
        Engine::Math::Float32From16(&z3, p.z3);

        std::cout << "Client: " 
            << static_cast<int>(p.PacketType) 
            << ", " 
            << x 
            << ", " 
            << y 
            << ", " 
            << z 
            << ", " 
            << x1
            << ", "
            << y1
            << ", "
            << z1
            << ", "
            << x2
            << ", "
            << y2
            << ", "
            << z2
            << ", "
            << x3
            << ", "
            << y3
            << ", "
            << z3
        << std::endl;
    }

}