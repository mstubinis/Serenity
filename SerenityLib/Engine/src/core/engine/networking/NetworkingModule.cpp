#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/networking/NetworkingModule.h>

using namespace std;
using namespace Engine::priv;


NetworkingModule::NetworkingModule() {

}
NetworkingModule::~NetworkingModule() {

}
void NetworkingModule::update(const float dt) {
    m_SocketManager.update(dt);
}