#include <core/engine/networking/SocketInterface.h>

using namespace std;

SocketStatus::Status SocketStatus::map_status(sf::Socket::Status sfmlStatus) {
    //return static_cast<SocketStatus::Status>(static_cast<unsigned int>(sfmlStatus));
    return static_cast<SocketStatus::Status>(sfmlStatus);
}