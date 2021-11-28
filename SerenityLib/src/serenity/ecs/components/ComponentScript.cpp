#include <serenity/ecs/components/ComponentScript.h>

ComponentScript::ComponentScript(Entity entity) 
    : m_Owner{ entity }
{

}
ComponentScript::ComponentScript(Entity entity, std::string_view scriptFilePath) 
    : ComponentScript{ entity }
{
    init(scriptFilePath);
}
ComponentScript::~ComponentScript() {

}
void ComponentScript::init(std::string_view scriptFilePath) {

}