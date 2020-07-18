#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/networking/SocketInterface.h>

using namespace std;

SocketStatus::Status SocketStatus::map_status(sf::Socket::Status sfmlStatus) {
    return (SocketStatus::Status)sfmlStatus;
}