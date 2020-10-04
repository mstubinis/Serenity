#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/math/MathCompression.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>

std::vector<glm::vec4> Material::m_MaterialProperities;

Handle Material::Checkers       = Handle{};
Handle Material::WhiteShadeless = Handle{};

constexpr std::array<MaterialDefaultPhysicsProperty, (size_t)MaterialPhysics::_TOTAL> MATERIAL_PROPERTIES { {
    { 5_uc, 5_uc, 5_uc, 128_uc, 1_uc },                // 0 - water
    { 8_uc, 8_uc, 8_uc, 246_uc, 1_uc },                // 1 - plastic or glass low
    { 13_uc, 13_uc, 13_uc, 234_uc, 1_uc },             // 2 - plastic high
    { 20_uc, 20_uc, 20_uc, 250_uc, 1_uc },             // 3 - glass or ruby high
    { 44_uc, 44_uc, 44_uc, 250_uc, 1_uc },             // 4 - diamond
    { 143_uc, 145_uc, 148_uc, 128_uc, 255_uc },        // 5 - iron
    { 243_uc, 162_uc, 137_uc, 229_uc, 255_uc },        // 6 - copper
    { 237_uc, 177_uc, 1_uc, 229_uc, 255_uc },          // 7 - gold
    { 233_uc, 235_uc, 235_uc, 191_uc, 255_uc },        // 8 - aluminium
    { 242_uc, 237_uc, 224_uc, 240_uc, 255_uc },        // 9 - silver
    { 1_uc, 1_uc, 2_uc, 115_uc, 1_uc },                // 10 - black leather
    { 81_uc, 56_uc, 13_uc, 81_uc, 1_uc },              // 11 - yellow paint MERL
    { 140_uc, 141_uc, 141_uc, 204_uc, 255_uc },        // 12 - chromium
    { 66_uc, 13_uc, 2_uc, 234_uc, 1_uc },              // 13 - red plastic MERL
    { 13_uc, 20_uc, 43_uc, 89_uc, 1_uc },              // 14 - blue rubber MERL
    { 169_uc, 210_uc, 217_uc, 229_uc, 255_uc },        // 15 - zinc
    { 255_uc, 51_uc, 1_uc, 229_uc, 128_uc },           // 16 - car paint orange
    { 7_uc, 7_uc, 7_uc, 25_uc, 1_uc },                 // 17 - skin
    { 11_uc, 11_uc, 11_uc, 204_uc, 1_uc },             // 18 - quartz
    { 28_uc, 28_uc, 28_uc, 229_uc, 1_uc },             // 19 - crystal
    { 5_uc, 5_uc, 5_uc, 204_uc, 1_uc },                // 20 - alcohol
    { 6_uc, 6_uc, 6_uc, 153_uc, 1_uc },                // 21 - milk
    { 10_uc, 10_uc, 10_uc, 247_uc, 1_uc },             // 22 - glass
    { 138_uc, 126_uc, 114_uc, 232_uc, 255_uc },        // 23 - titanium
    { 171_uc, 162_uc, 150_uc, 232_uc, 255_uc },        // 24 - platinum
    { 168_uc, 155_uc, 134_uc, 242_uc, 255_uc },        // 25 - nickel
} };

constexpr float ONE_OVER_255 = 0.003921568627451f;


constexpr auto DEFAULT_MATERIAL_BIND_FUNCTOR = [](Material* m) {
    auto&  components    = m->getComponents();
    size_t numComponents = components.size();
    size_t textureUnit   = 0;
    for (size_t i = 0; i < numComponents; ++i) {
        auto& component = components[i];
        if (component == true) {
            component.bind(i, textureUnit);
        }
    }
    Engine::Renderer::sendUniform1Safe("numComponents", (int)numComponents);
    Engine::Renderer::sendUniform1Safe("Shadeless", (int)m->shadeless());
    Engine::Renderer::sendUniform4Safe("Material_F0AndID", m->f0().r(), m->f0().g(), m->f0().b(), (float)m->id());
    Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesOne",
        (float)(m->glow()) * ONE_OVER_255,
        (float)(m->ao()) * ONE_OVER_255,
        (float)(m->metalness()) * ONE_OVER_255,
        (float)(m->smoothness()) * ONE_OVER_255
    );
    Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesTwo", (float)(m->alpha()) * ONE_OVER_255, (float)m->diffuseModel(), (float)m->specularModel(), 0.0f);
};

#pragma region Material

Material::Material()
    : Resource{ ResourceType::Material }
{
    Engine::priv::MaterialLoader::InitBase(*this);
    setCustomBindFunctor(DEFAULT_MATERIAL_BIND_FUNCTOR);
}
Material::Material(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular, const std::string& ao, const std::string& metalness, const std::string& smoothness)
    : Resource{ ResourceType::Material, name }
{
    auto d  = Engine::priv::MaterialLoader::LoadTextureDiffuse(diffuse);
    auto n  = Engine::priv::MaterialLoader::LoadTextureNormal(normal);
    auto g  = Engine::priv::MaterialLoader::LoadTextureGlow(glow);
    auto s  = Engine::priv::MaterialLoader::LoadTextureSpecular(specular);
    auto a  = Engine::priv::MaterialLoader::LoadTextureAO(ao);
    auto m  = Engine::priv::MaterialLoader::LoadTextureMetalness(metalness);
    auto sm = Engine::priv::MaterialLoader::LoadTextureSmoothness(smoothness);

    Engine::priv::MaterialLoader::Init(*this, d.second, n.second, g.second, s.second, a.second, m.second, sm.second);
    Engine::priv::InternalMaterialPublicInterface::Load(*this);
    setCustomBindFunctor(DEFAULT_MATERIAL_BIND_FUNCTOR);
}
Material::Material(const std::string& name, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness)
    : Resource{ ResourceType::Material, name }
{
    Engine::priv::MaterialLoader::Init(*this, diffuse, normal, glow, specular, ao, metalness, smoothness);
    Engine::priv::InternalMaterialPublicInterface::Load(*this);
    setCustomBindFunctor(DEFAULT_MATERIAL_BIND_FUNCTOR);
}
Material::Material(Material&& other) noexcept
    : Resource(std::move(other))
    , m_Components        { std::move(other.m_Components) }
    , m_CustomBindFunctor { std::move(other.m_CustomBindFunctor) }
    , m_DiffuseModel      { std::move(other.m_DiffuseModel) }
    , m_SpecularModel     { std::move(other.m_SpecularModel) }
    , m_Shadeless         { std::move(other.m_Shadeless) }
    , m_UpdatedThisFrame  { std::move(other.m_UpdatedThisFrame) }
    , m_F0Color           { std::move(other.m_F0Color) }
    , m_BaseGlow          { std::move(other.m_BaseGlow) }
    , m_BaseAO            { std::move(other.m_BaseAO) }
    , m_BaseMetalness     { std::move(other.m_BaseMetalness) }
    , m_BaseSmoothness    { std::move(other.m_BaseSmoothness) }
    , m_BaseAlpha         { std::move(other.m_BaseAlpha) }
    , m_ID                { std::move(other.m_ID) }
{}
Material& Material::operator=(Material&& other) noexcept {
    Resource::operator=(std::move(other));
    m_Components        = std::move(other.m_Components);
    m_CustomBindFunctor = std::move(other.m_CustomBindFunctor);
    m_DiffuseModel      = std::move(other.m_DiffuseModel);
    m_SpecularModel     = std::move(other.m_SpecularModel);
    m_Shadeless         = std::move(other.m_Shadeless);
    m_UpdatedThisFrame  = std::move(other.m_UpdatedThisFrame);
    m_F0Color           = std::move(other.m_F0Color);
    m_BaseGlow          = std::move(other.m_BaseGlow);
    m_BaseAO            = std::move(other.m_BaseAO);
    m_BaseMetalness     = std::move(other.m_BaseMetalness);
    m_BaseSmoothness    = std::move(other.m_BaseSmoothness);
    m_BaseAlpha         = std::move(other.m_BaseAlpha);
    m_ID                = std::move(other.m_ID);
    return *this;
}
Material::~Material() {
    Engine::priv::InternalMaterialPublicInterface::Unload(*this);
}
MaterialComponent* Material::internal_add_component_generic(MaterialComponentType type, Handle texture, Handle mask, Handle cubemap) {
    return &m_Components.emplace_back(type, texture, mask, cubemap);
}
void Material::internal_update_global_material_pool(bool addToDatabase) {
    //this data is kept around to be deferred to the lighting pass
    auto update_data = [this](glm::vec4& data) {
        data.r = Engine::Compression::pack3FloatsInto1FloatUnsigned(m_F0Color.r(), m_F0Color.g(), m_F0Color.b());
        data.g = (float)(m_BaseAlpha) * ONE_OVER_255;
        data.b = (float)m_SpecularModel;
        data.a = (float)m_DiffuseModel;
    };
    if (addToDatabase) {
        m_ID = (std::uint32_t)Material::m_MaterialProperities.size();
        auto& data = Material::m_MaterialProperities.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
        update_data(data);
    }else{
        update_data(Material::m_MaterialProperities[m_ID]);
    }
}


MaterialComponent& Material::addComponent(MaterialComponentType type, const std::string& textureFile, const std::string& maskFile, const std::string& cubemapFile) {
    auto texture = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get<Texture>(textureFile);
    if (!texture.first) {
        if (!textureFile.empty() && textureFile != "DEFAULT") {
            texture.second = Engine::Resources::addResource<Texture>(textureFile, true,
                (type == MaterialComponentType::Normal || type == MaterialComponentType::ParallaxOcclusion) ? ImageInternalFormat::RGB8 : ImageInternalFormat::SRGB8_ALPHA8
            );
            texture.first  = texture.second.get<Texture>();
        }else if (textureFile == "DEFAULT") {
            texture.second = Texture::Checkers;
            texture.first = texture.second.get<Texture>();
        }
    }
    auto mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    auto cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapFile);

    return *internal_add_component_generic(type, texture.second, mask.second, cubemap.second);
}
MaterialComponent& Material::addComponentDiffuse(const std::string& textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureDiffuse(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Diffuse, texture.second);
}
MaterialComponent& Material::addComponentNormal(const std::string& textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureNormal(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Normal, texture.second);
}
MaterialComponent& Material::addComponentGlow(const std::string& textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureGlow(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Glow, texture.second);
}
MaterialComponent& Material::addComponentSpecular(const std::string& textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureSpecular(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Specular, texture.second);
}
MaterialComponent& Material::addComponentAO(const std::string& textureFile, unsigned char baseValue){
    auto texture     = Engine::priv::MaterialLoader::LoadTextureAO(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::AO, texture.second);
    auto& layer      = component.layer(0);
    auto& _data2     = layer.data2();
    layer.setData2(0.0f, 1.0f, 1.0f, _data2.w);
    setAO(baseValue);
    return component;
}
MaterialComponent& Material::addComponentMetalness(const std::string& textureFile, unsigned char baseValue){
    auto texture     = Engine::priv::MaterialLoader::LoadTextureMetalness(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::Metalness, texture.second);
    auto& layer      = component.layer(0);
    auto& _data2     = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setMetalness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentSmoothness(const std::string& textureFile, unsigned char baseValue){
    auto texture     = Engine::priv::MaterialLoader::LoadTextureSmoothness(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::Smoothness, texture.second);
    auto& layer      = component.layer(0);
    auto& _data2     = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setSmoothness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentReflection(const std::string& cubemapName, const std::string& maskFile, float mixFactor){
    //add checks to see if texture was loaded already
    auto cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapName);
    auto mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap.first) {
        cubemap.second = Engine::Resources::getCurrentScene()->skybox()->texture();
    }
    auto& component = *internal_add_component_generic(MaterialComponentType::Reflection, Handle{});
    auto& layer     = component.layer(0);
    auto& _data2    = layer.data2();
    layer.setMask(mask.second);
    layer.setCubemap(cubemap.second);
    layer.setData2(mixFactor, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentRefraction(const std::string& cubemapName, const std::string& maskFile, float refractiveIndex, float mixFactor){
    //add checks to see if texture was loaded already
    auto cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapName);
    auto mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap.first) {
        cubemap.second = Engine::Resources::getCurrentScene()->skybox()->texture();
    }
    auto& component = *internal_add_component_generic(MaterialComponentType::Refraction, Handle{});
    auto& layer     = component.layer(0);
    auto& _data2    = layer.data2();
    layer.setMask(mask.second);
    layer.setCubemap(cubemap.second);
    layer.setData2(mixFactor, refractiveIndex, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentParallaxOcclusion(const std::string& textureFile, float heightScale){
    auto texture = Engine::priv::MaterialLoader::LoadTextureNormal(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::ParallaxOcclusion, texture.second);
    auto& layer      = component.layer(0);
    auto& _data2     = layer.data2();
    layer.setData2(heightScale, _data2.y, _data2.z, _data2.w);
    return component;
}
void Material::setShadeless(bool shadeless){
    m_Shadeless = shadeless;
    internal_update_global_material_pool(false);
}
void Material::setGlow(unsigned char glow){
    m_BaseGlow = glm::clamp(glow, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}

void Material::setF0Color(const Engine::color_vector_4& f0Color) {
    Material::setF0Color(f0Color.rc(), f0Color.gc(), f0Color.bc());
}
void Material::setF0Color(unsigned char r, unsigned char g, unsigned char b) {
    m_F0Color = Engine::color_vector_4(
        glm::clamp(r, 1_uc, 255_uc),
        glm::clamp(g, 1_uc, 255_uc),
        glm::clamp(b, 1_uc, 255_uc),
        255_uc
    );
    internal_update_global_material_pool(false);
}

void Material::setMaterialPhysics(MaterialPhysics materialPhysics){
    const auto& prop = MATERIAL_PROPERTIES[(size_t)materialPhysics];
    setF0Color(prop.r, prop.g, prop.b);
    setSmoothness(prop.smoothness);
    setMetalness(prop.metalness);
    internal_update_global_material_pool(false);
}
void Material::setSmoothness(unsigned char smoothness){
    m_BaseSmoothness = glm::clamp(smoothness, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}
void Material::setSpecularModel(SpecularModel specularModel){
    m_SpecularModel = specularModel;
    internal_update_global_material_pool(false);
}
void Material::setDiffuseModel(DiffuseModel diffuseModel){
    m_DiffuseModel = diffuseModel;
    internal_update_global_material_pool(false);
}
void Material::setAO(unsigned char ao){
    m_BaseAO = glm::clamp(ao, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}
void Material::setMetalness(unsigned char metalness){
    m_BaseMetalness = glm::clamp(metalness, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}
void Material::setAlpha(unsigned char alpha) {
    m_BaseAlpha = glm::clamp(alpha, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}
void Material::update(const float dt) {
    for (auto& component : m_Components) {
        component.update(dt);
    }
}
#pragma endregion