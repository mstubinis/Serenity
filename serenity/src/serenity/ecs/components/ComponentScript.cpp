#include <serenity/ecs/components/ComponentScript.h>
#include <serenity/system/Engine.h>
#include <filesystem>

ComponentScript::ComponentScript(Entity entity) 
    : m_Owner{ entity }
{

}
ComponentScript::ComponentScript(Entity entity, std::string_view scriptFilePathOrData) 
    : ComponentScript{ entity }
{
    init(scriptFilePathOrData);
}
ComponentScript::~ComponentScript() {

}
void ComponentScript::init(std::string_view scriptFilePathOrData) {
    const bool isFile = std::filesystem::is_regular_file(scriptFilePathOrData);
    m_LUAScript.runScript(scriptFilePathOrData, isFile, m_Owner);
    auto& editor = Engine::getEditor();
    if (editor.isEnabled()) {
        bool success = editor.addComponentScriptData(m_Owner, scriptFilePathOrData, isFile);
        assert(success);
    }
}