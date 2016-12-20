#include "Engine_ResourceBasic.h"

EngineResource::EngineResource(std::string name){ setName(name); }
EngineResource::~EngineResource(){ }
void EngineResource::setName(std::string name){ m_Name = name; }
std::string& EngineResource::name(){ return EngineResource::m_Name; }