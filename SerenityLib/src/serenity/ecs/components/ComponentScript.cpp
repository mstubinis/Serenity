#include <serenity/ecs/components/ComponentScript.h>
#include <serenity/system/Engine.h>

ComponentScript::ComponentScript(Entity entity) 
    : m_Owner{ entity }
{

}
ComponentScript::ComponentScript(Entity entity, std::string_view scriptFilePathOrData, bool fromFile) 
    : ComponentScript{ entity }
{
    init(scriptFilePathOrData, fromFile);
}
ComponentScript::~ComponentScript() {

}
void ComponentScript::init(std::string_view scriptFilePathOrData, bool fromFile) {
    m_LUAScript.runScript(scriptFilePathOrData, fromFile, m_Owner);
    auto& editor = Engine::priv::Core::m_Engine->m_Editor;
    if (editor.isEnabled()) {
        bool success = editor.addComponentScriptData(m_Owner, scriptFilePathOrData, fromFile);
        assert(success);
    }
}