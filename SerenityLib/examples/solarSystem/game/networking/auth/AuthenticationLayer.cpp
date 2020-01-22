#include "AuthenticationLayer.h"
#include "../server/Server.h"
#include "../packets/Packet.h"
#include "../packets/Packets.h"
#include "../../Helper.h"
#include "../../Core.h"
#include "../client/Client.h"
#include "../../map/Map.h"

#include "../../security/Security.h"

#include <iostream>
#include <random>
#include <boost/algorithm/string/replace.hpp>

using namespace std;

constexpr auto AUTH_TIME_ALLOTED = 15.0;

#pragma region AuthenticationInstance

AuthenticationLayer::AuthenticationInstance::AuthenticationInstance(Server& server, sf::TcpSocket* socket, const std::string& client_ip) : m_Server(server){
    m_Client_IP = client_ip;
    m_Timer     = 0.0;

    m_TcpSocket = NEW Engine::Networking::SocketTCP(socket);
    m_TcpSocket->setBlocking(false);
    m_Nonce = generate_nonce();

    std::cout << "Server: New client in authentication: " << m_Client_IP << std::endl;
}
AuthenticationLayer::AuthenticationInstance::~AuthenticationInstance() {
    SAFE_DELETE(m_TcpSocket);
}


const sf::Socket::Status AuthenticationLayer::AuthenticationInstance::send(Packet& packet) {
    sf::Packet sf_packet;
    packet.build(sf_packet);
    const auto status = m_TcpSocket->send(sf_packet);
    return status;
}
const sf::Uint32 AuthenticationLayer::AuthenticationInstance::generate_nonce() const {
    std::random_device device;
    std::mt19937 mt(device());
    std::uniform_int_distribution<sf::Uint32> distribution;
    const sf::Uint32 res = distribution(mt);
    return res;
}

void AuthenticationLayer::AuthenticationInstance::move_client_to_game(ServerClient& serverClient, ServerClientThread& thread, const string& real_username) {
    {
        std::lock_guard<std::mutex> lock(m_Server.m_Mutex);
        thread.m_Clients.emplace(m_Client_IP, &serverClient);
        thread.m_Active.store(1, std::memory_order_relaxed);
        if (!m_Server.m_OwnerClient) {
            m_Server.m_OwnerClient = &serverClient;
        }
    }

    m_Server.assign_username_to_client(serverClient, real_username);

    PacketConnectionAccepted pOut;
    Map* map = m_Server.m_MapSpecificData.m_Map;

    //a client wants to connect to the server
    serverClient.m_Validated = true;
    pOut.PacketType = PacketType::Server_To_Client_Accept_Connection;
    pOut.game_mode_type = static_cast<unsigned char>(Server::SERVER_HOST_DATA.getGameplayMode().getGameplayMode());
    pOut.already_connected_players = "";
    pOut.allowed_ships = Helper::Stringify(Server::SERVER_HOST_DATA.getAllowedShips(), ',');
    pOut.lobby_time_left = Server::SERVER_HOST_DATA.getCurrentLobbyTime();
    if (serverClient.m_Username == m_Server.m_Core.getClient()->username()) {
        pOut.is_host = true;
    }
    for (auto& clientThread : m_Server.m_Threads) {
        for (auto& c : clientThread->m_Clients) {
            if (!c.second->m_Username.empty() && c.second->m_Username != serverClient.m_Username) {
                pOut.already_connected_players += c.second->m_Username + ",";
            }
        }
    }
    pOut.already_connected_players += std::to_string(serverClient.m_ID) + ",";
    if (!pOut.already_connected_players.empty())
        pOut.already_connected_players.pop_back();

    pOut.map_file_name = map->getFilename();
    pOut.map_name = map->name();

    m_Server.send_to_client(serverClient, pOut);
    std::cout << "Server: Approving: " + real_username + "'s authentication" << std::endl;

    //now send the client info about the gameplay mode
    if (serverClient.m_Username != m_Server.m_Core.getClient()->username()) { //as the host, we already have info about it
        PacketGameplayModeInfo info = Server::SERVER_HOST_DATA.getGameplayMode().serialize();
        info.PacketType = PacketType::Server_To_Client_Request_GameplayMode;
        m_Server.send_to_client(serverClient, info);
    }

    PacketMessage packetClientJustJoined;
    packetClientJustJoined.name = serverClient.m_Username;
    packetClientJustJoined.PacketType = PacketType::Server_To_Client_Client_Joined_Server;
    m_Server.send_to_all(packetClientJustJoined);
}
void AuthenticationLayer::AuthenticationInstance::end_this_instance() {
    m_TcpSocket = nullptr;
    m_Timer     = AUTH_TIME_ALLOTED;
}
void AuthenticationLayer::AuthenticationInstance::end_auth_process(const string& real_username) {
    ServerClientThread* leastThread = m_Server.getNextAvailableClientThread();
    if (!leastThread->m_Clients.count(m_Client_IP)) {
        ServerClient* serverClient = NEW ServerClient(m_Client_IP, m_Server, m_Server.m_Core, *m_TcpSocket);
        move_client_to_game(*serverClient, *leastThread, real_username);
        end_this_instance();
    }
}
void AuthenticationLayer::AuthenticationInstance::update(const double& dt) {
    if (!m_TcpSocket)
        return;
    sf::Packet sf_packet;
    const auto status = m_TcpSocket->receive(sf_packet);
    m_Timer += dt;
    if (status == sf::Socket::Done) {
        auto basePacket_Ptr = std::unique_ptr<Packet>(std::move(Packet::getPacket(sf_packet)));
        const auto valid = basePacket_Ptr->validate(sf_packet);
        if (basePacket_Ptr && valid) {
            Packet& pIn = *basePacket_Ptr;
            // Data extracted successfully...
            switch (pIn.PacketType) {
                case PacketType::Client_To_Server_Auth_Part_1: {
                    PacketAuthStep1& pI = *static_cast<PacketAuthStep1*>(&pIn);
                    pI.PacketType       = PacketType::Server_To_Client_Auth_Part_1;
                    pI.server_nonce     = m_Nonce;
                    m_SecretKey         = pI.secret_key;
                    send(pI);
                    break;
                }
                case PacketType::Client_To_Server_Auth_Part_2: {
                    PacketAuthStep2& pI   = *static_cast<PacketAuthStep2*>(&pIn);
                    string real_username  = Security::decrypt_aes(pI.username, (pI.server_nonce / 2) + (pI.client_nonce / 2), m_SecretKey);
                    string real_password  = Security::decrypt_aes(pI.password, ((pI.server_nonce / 4) + 1) + ((pI.client_nonce / 3) + 1), m_SecretKey);
                    if (Server::PERSISTENT_INFO == true) {
                        // now retrieve server id
                        const auto& serverName = Server::PERSISTENT_INFO.getServerName();
                        int server_id = Server::DATABASE.get_server_id(serverName);
                        if (server_id != -1) {
                            //now get the account corresponding with this server id
                            auto res_acc = Server::DATABASE.get_account(real_username, server_id);

                            auto hashed_pw_in_database = (res_acc.get(0, "Password"));
                            auto salt_in_database      = (res_acc.get(0, "Salt"));

                            string pepper            = "gjsuwmejfidlskdfig59dgfkkdfiejrmfj";
                            string peppered_password = Security::sha256(pepper + real_password);
                            string hashed_password   = Security::argon2id(salt_in_database, real_password, 1, 2, 1 << 16, Database::CONST_SALT_LENGTH_MAX, Database::CONST_USERNAME_PASSWORD_HASH_LENGTH_MAX);

                            if (hashed_password == hashed_pw_in_database) {
                                //valid credentials
                                std::cout << "Server: Approving: " + real_username + "'s connection - credentials approved" << std::endl;
                            }else {
                                //invalid credentials
                                PacketMessage pOut;
                                pOut.PacketType = PacketType::Server_To_Client_Reject_Connection;
                                std::cout << "Server: Rejecting: " + real_username + "'s connection - invalid username and or password" << std::endl;
                                send(pOut);

                                end_this_instance();
                                return;
                            }
                        }

                    }

                    auto valid                       = m_Server.isValidName(real_username);
                    if (valid) {
                        end_auth_process(real_username);
                    }else{
                        PacketMessage pOut;
                        pOut.PacketType = PacketType::Server_To_Client_Reject_Connection;
                        std::cout << "Server: Rejecting: " + real_username + "'s connection - invalid username" << std::endl;
                        send(pOut);

                        end_this_instance();
                        return;
                    }
                    break;
                }
            }
        }
    }
}

#pragma endregion

#pragma region AuthenticationLayer

AuthenticationLayer::AuthenticationLayer(Server& server) : m_Server(server) {

}
AuthenticationLayer::~AuthenticationLayer() {

}

const bool AuthenticationLayer::allow_host_to_pass(const string& real_username) {
    for (auto& instance_itr : m_Instances) {
        auto& instance = instance_itr.second;
        auto& key = instance_itr.first;
        if (key.find("127.0.0.1") != string::npos) {
            ServerClientThread* leastThread = m_Server.getNextAvailableClientThread();
            ServerClient* serverClient = NEW ServerClient(key, m_Server, m_Server.m_Core, *instance.m_TcpSocket);
            //std::cout << "Host is connecting to own server\n";
            instance.move_client_to_game(*serverClient, *leastThread, real_username);
            instance.end_this_instance();
            return true;
        }
    }
    return false;
}

const bool AuthenticationLayer::create_instance(const string& client_ip, sf::TcpSocket* socket) {
    if (m_Instances.count(client_ip)) {
        return false;
    }
    m_Instances.emplace(  std::piecewise_construct,  std::forward_as_tuple(client_ip),  std::forward_as_tuple(m_Server, socket, client_ip)  );
    return true;
}

void AuthenticationLayer::update(const double& dt) {
    auto it = m_Instances.begin();
    while (it != m_Instances.end()) {
        auto& instance = (*it).second;
        instance.update(dt);
        if (instance.m_Timer > AUTH_TIME_ALLOTED) {
            it = m_Instances.erase(it);
        }else{
            it++;
        }
    }
}

#pragma endregion