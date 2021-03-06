#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/networking/NetworkingModule.h>

using namespace Engine::priv;

void NetworkingModule::update(const float dt) {
    m_SocketManager.update(dt);
}