#include "LightProbe.h"

using namespace std;

LightProbe::LightProbe(const ProbeType::Type type, const glm::vec3 position, Scene* scene){
    m_ProbeType  = type;
    m_ProbeType  = ProbeType::Static;
    m_Active     = true;
}
LightProbe::~LightProbe() {

}

void LightProbe::update(const double& dt) {

}

void LightProbe::activate(const bool active) {
    m_Active = active;
}
void LightProbe::deactivate() {
    m_Active = false;
}
void LightProbe::setProbeType(const ProbeType::Type& type) {
    m_ProbeType = type;
}
const LightProbe::ProbeType::Type& LightProbe::getProbeType() const {
    return m_ProbeType;
}