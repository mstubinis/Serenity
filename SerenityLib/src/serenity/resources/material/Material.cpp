#include <serenity/resources/material/Material.h>
#include <serenity/resources/material/MaterialLoader.h>
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/system/Engine.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/math/MathCompression.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Skybox.h>

std::vector<glm::vec4>   Material::m_MaterialProperities;
Handle                   Material::Checkers                = {};
Handle                   Material::WhiteShadeless          = {};

namespace {
    constexpr std::array<MaterialDefaultPhysicsProperty, MaterialPhysics::_TOTAL> MATERIAL_PROPERTIES{ {
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
}
#pragma region Material

Material::Material()
    : Resource{ ResourceType::Material }
{
    Engine::priv::MaterialLoader::InitBase(*this);
}
Material::Material(std::string_view name, std::string_view diffuse, std::string_view normal, std::string_view glow, std::string_view specular, std::string_view ao, std::string_view metalness, std::string_view smoothness)
    : Resource{ ResourceType::Material, name }
{
    auto d  = Engine::priv::MaterialLoader::LoadTextureDiffuse(diffuse);
    auto n  = Engine::priv::MaterialLoader::LoadTextureNormal(normal);
    auto g  = Engine::priv::MaterialLoader::LoadTextureGlow(glow);
    auto s  = Engine::priv::MaterialLoader::LoadTextureSpecular(specular);
    auto m  = Engine::priv::MaterialLoader::LoadTextureMetalness(metalness);
    auto sm = Engine::priv::MaterialLoader::LoadTextureSmoothness(smoothness);
    auto a  = Engine::priv::MaterialLoader::LoadTextureAO(ao);

    Engine::priv::MaterialLoader::Init(*this, d, n, g, s, a, m, sm);
    Engine::priv::PublicMaterial::Load(*this);
}
Material::Material(std::string_view name, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness)
    : Resource{ ResourceType::Material, name }
{
    Engine::priv::MaterialLoader::Init(*this, diffuse, normal, glow, specular, ao, metalness, smoothness);
    Engine::priv::PublicMaterial::Load(*this);
}
Material::Material(Material&& other) noexcept
    : Resource{ std::move(other) }
    , m_Components       { std::move(other.m_Components) }
    , m_DiffuseModel     { std::move(other.m_DiffuseModel) }
    , m_SpecularModel    { std::move(other.m_SpecularModel) }
    , m_Shadeless        { std::move(other.m_Shadeless) }
    , m_F0Color          { std::move(other.m_F0Color) }
    , m_BaseGlow         { std::move(other.m_BaseGlow) }
    , m_BaseMetalness    { std::move(other.m_BaseMetalness) }
    , m_BaseSmoothness   { std::move(other.m_BaseSmoothness) }
    , m_BaseAO           { std::move(other.m_BaseAO) }
    , m_BaseAlpha        { std::move(other.m_BaseAlpha) }
    , m_ID               { std::move(other.m_ID) }
    , m_UpdatedThisFrame { std::move(other.m_UpdatedThisFrame) }
{}
Material& Material::operator=(Material&& other) noexcept {
    Resource::operator=(std::move(other));
    m_Components       = std::move(other.m_Components);
    m_DiffuseModel     = std::move(other.m_DiffuseModel);
    m_SpecularModel    = std::move(other.m_SpecularModel);
    m_Shadeless        = std::move(other.m_Shadeless);
    m_F0Color          = std::move(other.m_F0Color);
    m_BaseGlow         = std::move(other.m_BaseGlow);
    m_BaseMetalness    = std::move(other.m_BaseMetalness);
    m_BaseSmoothness   = std::move(other.m_BaseSmoothness);
    m_BaseAO           = std::move(other.m_BaseAO);
    m_BaseAlpha        = std::move(other.m_BaseAlpha);
    m_ID               = std::move(other.m_ID);
    m_UpdatedThisFrame = std::move(other.m_UpdatedThisFrame);
    return *this;
}
Material::~Material() {
    Engine::priv::PublicMaterial::Unload(*this);
}
MaterialComponent* Material::internal_add_component_generic(MaterialComponentType type, Handle texture, Handle mask, Handle cubemap) {
    if (m_Components.size() <= type) {
        m_Components.resize(type + 1);
    }
    if (m_Components[type].getType() == type) {
        return &m_Components[type];
    }
    m_Components[type] = MaterialComponent{ type, texture, mask, cubemap };
    return &m_Components[type];
}
void Material::internal_update_global_material_pool(bool addToDatabase) noexcept {
    //this data is kept around to be deferred to the lighting pass
    auto update_data = [this](glm::vec4& materialData) {
        materialData.r = Engine::Compression::pack3FloatsInto1FloatUnsigned(m_F0Color.r(), m_F0Color.g(), m_F0Color.b());
        materialData.g = float(m_BaseAlpha) * (1.0f / 255.0f);
        materialData.b = float(m_SpecularModel);
        materialData.a = float(m_DiffuseModel);
    };
    if (addToDatabase) {
        m_ID       = MaterialID(Material::m_MaterialProperities.size());
        auto& data = Material::m_MaterialProperities.emplace_back(0.0f, 0.0f, 0.0f, 0.0f);
        update_data(data);
    } else {
        update_data(Material::m_MaterialProperities[m_ID]);
    }
}
MaterialComponent& Material::addComponent(MaterialComponentType type, std::string_view textureFile, std::string_view maskFile, std::string_view cubemapFile) {
    auto texture = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourceModule.get<Texture>(textureFile);
    if (!texture.m_Resource) {
        if (!textureFile.empty()) {
            ImageInternalFormat fmt = (type == MaterialComponentType::Normal || type == MaterialComponentType::ParallaxOcclusion) ? ImageInternalFormat::RGB8 : ImageInternalFormat::SRGB8_ALPHA8;
            texture.m_Handle = Engine::Resources::addResource<Texture>(textureFile, true, fmt, TextureType::Texture2D);
        } else {
            texture.m_Handle = Texture::Checkers;
        }
        texture.m_Resource = texture.m_Handle.get<Texture>();
    }
    auto mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    auto cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapFile);

    return *internal_add_component_generic(type, texture, mask, cubemap);
}
MaterialComponent& Material::addComponentDiffuse(std::string_view textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureDiffuse(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Diffuse, texture);
}
MaterialComponent& Material::addComponentNormal(std::string_view textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureNormal(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Normal, texture);
}
MaterialComponent& Material::addComponentGlow(std::string_view textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureGlow(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Glow, texture);
}
MaterialComponent& Material::addComponentSpecular(std::string_view textureFile){
    auto texture = Engine::priv::MaterialLoader::LoadTextureSpecular(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Specular, texture);
}
MaterialComponent& Material::addComponentAO(std::string_view textureFile, uint8_t baseValue){
    auto texture     = Engine::priv::MaterialLoader::LoadTextureAO(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::AO, texture);
    auto& layer      = component.getLayer(0);
    auto& data2_     = layer.getMaterialLayerMiscData();
    layer.setMiscData(0.0f, 1.0f, 1.0f, data2_.aMultiplier);
    setAO(baseValue);
    return component;
}
MaterialComponent& Material::addComponentMetalness(std::string_view textureFile, uint8_t baseValue){
    auto texture     = Engine::priv::MaterialLoader::LoadTextureMetalness(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::Metalness, texture);
    auto& layer      = component.getLayer(0);
    auto& data2_     = layer.getMaterialLayerMiscData();
    layer.setMiscData(0.01f, 0.99f, 1.0f, data2_.aMultiplier);
    setMetalness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentSmoothness(std::string_view textureFile, uint8_t baseValue){
    auto texture     = Engine::priv::MaterialLoader::LoadTextureSmoothness(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::Smoothness, texture);
    auto& layer      = component.getLayer(0);
    auto& data2_     = layer.getMaterialLayerMiscData();
    layer.setMiscData(0.01f, 0.99f, 1.0f, data2_.aMultiplier);
    setSmoothness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentReflection(std::string_view cubemapName, const std::string& maskFile, float mixFactor){
    //add checks to see if texture was loaded already
    auto cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapName);
    auto mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap.m_Resource) {
        cubemap.m_Handle = Engine::Resources::getCurrentScene()->skybox()->cubemap();
    }
    auto& component = *internal_add_component_generic(MaterialComponentType::Reflection, Handle{});
    auto& layer     = component.getLayer(0);
    auto& data2_    = layer.getMaterialLayerMiscData();
    layer.setMask(mask);
    layer.setCubemap(cubemap);
    layer.setMiscData(mixFactor, data2_.gMultiplier, data2_.bMultiplier, data2_.aMultiplier);
    return component;
}
MaterialComponent& Material::addComponentRefraction(std::string_view cubemapName, const std::string& maskFile, float refractiveIndex, float mixFactor){
    //add checks to see if texture was loaded already
    auto cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapName);
    auto mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap.m_Resource) {
        cubemap.m_Handle = Engine::Resources::getCurrentScene()->skybox()->cubemap();
    }
    auto& component = *internal_add_component_generic(MaterialComponentType::Refraction, Handle{});
    auto& layer     = component.getLayer(0);
    auto& data2_    = layer.getMaterialLayerMiscData();
    layer.setMask(mask);
    layer.setCubemap(cubemap);
    layer.setMiscData(mixFactor, refractiveIndex, data2_.bMultiplier, data2_.aMultiplier);
    return component;
}
MaterialComponent& Material::addComponentParallaxOcclusion(std::string_view textureFile, float heightScale){
    auto texture     = Engine::priv::MaterialLoader::LoadTextureNormal(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::ParallaxOcclusion, texture);
    auto& layer      = component.getLayer(0);
    auto& data2_     = layer.getMaterialLayerMiscData();
    layer.setMiscData(heightScale, data2_.gMultiplier, data2_.bMultiplier, data2_.aMultiplier);
    return component;
}
void Material::setShadeless(bool shadeless){
    m_Shadeless = shadeless;
    internal_update_global_material_pool(false);
}
void Material::setGlow(uint8_t glow){
    m_BaseGlow = glm::clamp(glow, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}

void Material::setF0Color(uint8_t r, uint8_t g, uint8_t b) noexcept {
    m_F0Color = Engine::color_vector_4{ glm::clamp(r, 1_uc, 255_uc), glm::clamp(g, 1_uc, 255_uc), glm::clamp(b, 1_uc, 255_uc), 255_uc };
    internal_update_global_material_pool(false);
}

void Material::setMaterialPhysics(MaterialPhysics materialPhysics){
    const auto& prop = MATERIAL_PROPERTIES[materialPhysics];
    setF0Color(prop.r, prop.g, prop.b);
    setSmoothness(prop.smoothness);
    setMetalness(prop.metalness);
    internal_update_global_material_pool(false);
}
void Material::setSmoothness(uint8_t smoothness){
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
void Material::setAO(uint8_t ao){
    m_BaseAO = glm::clamp(ao, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}
void Material::setMetalness(uint8_t metalness){
    m_BaseMetalness = glm::clamp(metalness, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}
void Material::setAlpha(uint8_t alpha) {
    m_BaseAlpha = glm::clamp(alpha, 1_uc, 255_uc);
    internal_update_global_material_pool(false);
}
void Material::update(const float dt) {
    for (auto& component : m_Components) {
        component.update(dt);
    }
}
#pragma endregion