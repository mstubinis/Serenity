#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lua/LuaModule.h>

using namespace Engine::priv;

LUAModule* manager = nullptr;

LUAModule::LUAModule() {
    manager = this;
}
LUAModule::~LUAModule() {


}

void LUAModule::init() {

}
const LUABinder& LUAModule::getBinder() const {
    return m_Binder;
}


const Engine::priv::LUABinder& Engine::priv::getLUABinder() {
    return manager->getBinder();
}
