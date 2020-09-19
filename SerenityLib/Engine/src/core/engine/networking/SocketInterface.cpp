#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/SocketInterface.h>

SocketStatus::Status SocketStatus::map_status(sf::Socket::Status sfmlStatus) {
    return (SocketStatus::Status)sfmlStatus;
}