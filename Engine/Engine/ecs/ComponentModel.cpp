#include "ComponentModel.h"

using namespace Engine;
using namespace std;

/*

epriv::ComponentModelSystem::ComponentModelSystem(){
}
epriv::ComponentModelSystem::~ComponentModelSystem(){ 
}

static void _calculateRenderCheck(ComponentModel& m, Camera* camera) {
    auto& body = *(m.owner()->getComponent<ComponentBody>());
    for (auto meshInstance : m.models) {
        auto& _meshInstance = *meshInstance;
        auto pos = body.position() + _meshInstance.position();
        //per mesh instance radius instead?
        uint sphereTest = camera->sphereIntersectTest(pos, m._radius);                //per mesh instance radius instead?
        if (!_meshInstance.visible() || sphereTest == 0 || camera->getDistance(pos) > m._radius * 1100.0f) {
            _meshInstance.setPassedRenderCheck(false);
            continue;
        }
        _meshInstance.setPassedRenderCheck(true);
    }
}
static void _defaultUpdate(vector<ComponentBaseClass*>& vec, Camera* camera) {
    for (uint j = 0; j < vec.size(); ++j) {
        auto& model = *(ComponentModel*)vec[j];
        for (uint i = 0; i < model.models.size(); ++i) {
            auto& pair = *model.models[i];
            if (pair.mesh()) {
                //TODO: implement parent->child relationship...?
                componentManager->m_i->_performTransformation(nullptr, pair.position(), pair.orientation(), pair.getScale(), pair.model());
                _calculateRenderCheck(model, camera);
            }
        }
    }
}
void epriv::ComponentModelSystem::update(const float& dt) {
    auto* camera = Resources::getCurrentScene()->getActiveCamera();
    uint slot = Components::getSlot<ComponentModel>();
    auto& v = ComponentManager::m_ComponentVectorsScene[slot];
    auto split = epriv::threading::splitVector(v);
    for (auto vec : split) {
        epriv::threading::addJob(_defaultUpdate, vec, camera);
    }
    epriv::threading::waitForAll();   
}


*/