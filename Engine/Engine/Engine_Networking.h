#pragma once
#ifndef ENGINE_NETWORKING_H
#define ENGINE_NETWORKING_H

#include <SFML/Network.hpp>
#include <memory>
typedef unsigned int uint;
typedef unsigned short ushort;

namespace Engine{namespace Networking{

    class ISocket{
        public:
            virtual void setBlocking(bool) = 0;
            virtual const bool isBlocking() = 0;
            virtual const sf::Socket& socket() = 0;
            virtual const ushort localPort() = 0;
    };
    class SocketTCP: public ISocket{
        private: class impl; std::unique_ptr<impl> m_i;
        public:
            SocketTCP(const uint port, const std::string& ip = "",uint timeout = 0);
            ~SocketTCP();

            const sf::TcpSocket& socket();
            const std::string& ip();    
            const ushort remotePort();
            const ushort localPort();

            void connect();
            void disconnect();
            void setBlocking(bool);
            const bool isBlocking();

            void send(sf::Packet& packet);
            void send(const void* data, size_t size);
            void send(const void* data, size_t size,size_t& sent);

            void receive(sf::Packet& packet);
            void receive(void* data, size_t size,size_t& received);
    };
    class SocketUDP: public ISocket{
        private: class impl; std::unique_ptr<impl> m_i;
        public:
            SocketUDP(const uint port);
            ~SocketUDP();

            const sf::UdpSocket& socket();
            const ushort localPort();
            void setBlocking(bool);
            const bool isBlocking();

            void bind(const std::string& _ip = "");
            void unbind();
            void send(sf::Packet& packet,const std::string& _ip = "");
            void send(const void* data, size_t size,const std::string& _ip = "");

            void receive(sf::Packet& packet,const std::string& _ip = "");
            void receive(void* data, size_t size,size_t& received,const std::string& _ip = "");
    };
    class ListenerTCP: public ISocket{
        private: class impl; std::unique_ptr<impl> m_i;
        public:
            ListenerTCP(const uint port,const std::string& ip = "");
            ~ListenerTCP();

            const sf::TcpListener& socket();
            const std::string& ip();
            const ushort localPort();
            void setBlocking(bool);
            const bool isBlocking();
            void close();
            void accept(SocketTCP&);
            void accept(SocketTCP*);
            void listen();
    };
};};
#endif