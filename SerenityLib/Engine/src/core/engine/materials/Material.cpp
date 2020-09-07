#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>

std::vector<glm::vec4> Material::m_MaterialProperities;

Material* Material::Checkers       = nullptr;
Material* Material::WhiteShadeless = nullptr;

constexpr std::array<std::tuple<unsigned char, unsigned char, unsigned char, unsigned char, unsigned char>, (size_t)MaterialPhysics::_TOTAL> MATERIAL_PROPERTIES {
    std::make_tuple(5_uc, 5_uc, 5_uc, 128_uc, 1_uc),                // 0 - water
    std::make_tuple(8_uc, 8_uc, 8_uc, 246_uc, 1_uc),                // 1 - plastic or glass low
    std::make_tuple(13_uc, 13_uc, 13_uc, 234_uc, 1_uc),             // 2 - plastic high
    std::make_tuple(20_uc, 20_uc, 20_uc, 250_uc, 1_uc),             // 3 - glass or ruby high
    std::make_tuple(44_uc, 44_uc, 44_uc, 250_uc, 1_uc),             // 4 - diamond
    std::make_tuple(143_uc, 145_uc, 148_uc, 128_uc, 255_uc),        // 5 - iron
    std::make_tuple(243_uc, 162_uc, 137_uc, 229_uc, 255_uc),        // 6 - copper
    std::make_tuple(237_uc, 177_uc, 1_uc, 229_uc, 255_uc),          // 7 - gold
    std::make_tuple(233_uc, 235_uc, 235_uc, 191_uc, 255_uc),        // 8 - aluminium
    std::make_tuple(242_uc, 237_uc, 224_uc, 240_uc, 255_uc),        // 9 - silver
    std::make_tuple(1_uc, 1_uc, 2_uc, 115_uc, 1_uc),                // 10 - black leather
    std::make_tuple(81_uc, 56_uc, 13_uc, 81_uc, 1_uc),              // 11 - yellow paint MERL
    std::make_tuple(140_uc, 141_uc, 141_uc, 204_uc, 255_uc),        // 12 - chromium
    std::make_tuple(66_uc, 13_uc, 2_uc, 234_uc, 1_uc),              // 13 - red plastic MERL
    std::make_tuple(13_uc, 20_uc, 43_uc, 89_uc, 1_uc),              // 14 - blue rubber MERL
    std::make_tuple(169_uc, 210_uc, 217_uc, 229_uc, 255_uc),        // 15 - zinc
    std::make_tuple(255_uc, 51_uc, 1_uc, 229_uc, 128_uc),           // 16 - car paint orange
    std::make_tuple(7_uc, 7_uc, 7_uc, 25_uc, 1_uc),                 // 17 - skin
    std::make_tuple(11_uc, 11_uc, 11_uc, 204_uc, 1_uc),             // 18 - quartz
    std::make_tuple(28_uc, 28_uc, 28_uc, 229_uc, 1_uc),             // 19 - crystal
    std::make_tuple(5_uc, 5_uc, 5_uc, 204_uc, 1_uc),                // 20 - alcohol
    std::make_tuple(6_uc, 6_uc, 6_uc, 153_uc, 1_uc),                // 21 - milk
    std::make_tuple(10_uc, 10_uc, 10_uc, 247_uc, 1_uc),             // 22 - glass
    std::make_tuple(138_uc, 126_uc, 114_uc, 232_uc, 255_uc),        // 23 - titanium
    std::make_tuple(171_uc, 162_uc, 150_uc, 232_uc, 255_uc),        // 24 - platinum
    std::make_tuple(168_uc, 155_uc, 134_uc, 242_uc, 255_uc),        // 25 - nickel
};

constexpr float ONE_OVER_255 = 0.003921568627451f;

namespace Engine::priv {
    struct DefaultMaterialBindFunctor { void operator()(Material* material_ptr) const {
        auto& material       = *material_ptr;
        size_t numComponents = material.m_Components.size();
        size_t textureUnit   = 0;

        for (size_t i = 0; i < numComponents; ++i) {
            if (material.m_Components[i]) {
                auto& component = *material.m_Components[i];
                component.bind(i, textureUnit);
            }
        }
        Engine::Renderer::sendUniform1Safe("numComponents", (int)numComponents);
        Engine::Renderer::sendUniform1Safe("Shadeless", (int)material.m_Shadeless);
        Engine::Renderer::sendUniform4Safe("Material_F0AndID",
            material.m_F0Color.r(),
            material.m_F0Color.g(),
            material.m_F0Color.b(),
            (float)material.m_ID
        );
        Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesOne",
            (float)(material.m_BaseGlow) * ONE_OVER_255,
            (float)(material.m_BaseAO) * ONE_OVER_255,
            (float)(material.m_BaseMetalness) * ONE_OVER_255,
            (float)(material.m_BaseSmoothness) * ONE_OVER_255
        );
        Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesTwo",
            (float)(material.m_BaseAlpha) * ONE_OVER_255,
            (float)material.m_DiffuseModel,
            (float)material.m_SpecularModel,
            0.0f
        );
    }};
};

#pragma region Material

Material::Material(const std::string& name, const std::string& diffuse, const std::string& normal, const std::string& glow, const std::string& specular, const std::string& ao, const std::string& metalness, const std::string& smoothness) : Resource(ResourceType::Material, name){
    Texture* d  = Engine::priv::MaterialLoader::LoadTextureDiffuse(diffuse);
    Texture* n  = Engine::priv::MaterialLoader::LoadTextureNormal(normal);
    Texture* g  = Engine::priv::MaterialLoader::LoadTextureGlow(glow);
    Texture* s  = Engine::priv::MaterialLoader::LoadTextureSpecular(specular);
    Texture* a  = Engine::priv::MaterialLoader::LoadTextureAO(ao);
    Texture* m  = Engine::priv::MaterialLoader::LoadTextureMetalness(metalness);
    Texture* sm = Engine::priv::MaterialLoader::LoadTextureSmoothness(smoothness);

    Engine::priv::MaterialLoader::Init(*this, d, n, g, s, a, m, sm);
    Engine::priv::InternalMaterialPublicInterface::Load(*this);
    setCustomBindFunctor(Engine::priv::DefaultMaterialBindFunctor());
}
Material::Material() : Resource(ResourceType::Material) {
    Engine::priv::MaterialLoader::InitBase(*this);
    setCustomBindFunctor(Engine::priv::DefaultMaterialBindFunctor());
}
Material::Material(const std::string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness) : Resource(ResourceType::Material, name){
    Engine::priv::MaterialLoader::Init(*this, diffuse, normal, glow, specular, ao, metalness, smoothness);
    Engine::priv::InternalMaterialPublicInterface::Load(*this);
    setCustomBindFunctor(Engine::priv::DefaultMaterialBindFunctor());
}
Material::~Material(){
    Engine::priv::InternalMaterialPublicInterface::Unload(*this);
    SAFE_DELETE_VECTOR(m_Components);
}
MaterialComponent* Material::internal_add_component_generic(MaterialComponentType type, Texture* texture, Texture* mask, Texture* cubemap) {
    MaterialComponent* newMaterialComponent = NEW MaterialComponent(type, texture, mask, cubemap);
    return m_Components.emplace_back(newMaterialComponent);
}
void Material::internal_update_global_material_pool(bool addToDatabase) {
    //this data is kept around to be deferred to the lighting pass
    auto update_data = [this](glm::vec4& data) {
        data.r = Engine::Math::pack3FloatsInto1FloatUnsigned(m_F0Color.r(), m_F0Color.g(), m_F0Color.b());
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
    Texture* texture = Engine::priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(textureFile);
    if (!texture) {
        if (!textureFile.empty() && textureFile != "DEFAULT") {
            if (type == MaterialComponentType::Normal || type == MaterialComponentType::ParallaxOcclusion) {
                texture = NEW Texture(textureFile, true, ImageInternalFormat::RGB8);
            }else{
                texture = NEW Texture(textureFile, true, ImageInternalFormat::SRGB8_ALPHA8);
            }
            Engine::priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }else if (textureFile == "DEFAULT") {
            texture = Texture::Checkers;
        }else{
        }
    }
    Texture* mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    Texture* cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapFile);

    return *internal_add_component_generic(type, texture, mask, cubemap);
}
MaterialComponent& Material::addComponentDiffuse(const std::string& textureFile){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureDiffuse(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Diffuse, texture);
}
MaterialComponent& Material::addComponentNormal(const std::string& textureFile){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureNormal(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Normal, texture);
}
MaterialComponent& Material::addComponentGlow(const std::string& textureFile){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureGlow(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Glow, texture);
}
MaterialComponent& Material::addComponentSpecular(const std::string& textureFile){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureSpecular(textureFile);
    return *internal_add_component_generic(MaterialComponentType::Specular, texture);
}
MaterialComponent& Material::addComponentAO(const std::string& textureFile, unsigned char baseValue){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureAO(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::AO, texture);
    auto& layer      = component.layer(0);
    auto& _data2     = layer.data2();
    layer.setData2(0.0f, 1.0f, 1.0f, _data2.w);
    setAO(baseValue);
    return component;
}
MaterialComponent& Material::addComponentMetalness(const std::string& textureFile, unsigned char baseValue){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureMetalness(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::Metalness, texture);
    auto& layer      = component.layer(0);
    auto& _data2     = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setMetalness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentSmoothness(const std::string& textureFile, unsigned char baseValue){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureSmoothness(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::Smoothness, texture);
    auto& layer      = component.layer(0);
    auto& _data2     = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setSmoothness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentReflection(const std::string& cubemapName, const std::string& maskFile, float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapName);
    Texture* mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap) {
        cubemap = Engine::Resources::getCurrentScene()->skybox()->texture();
    }
    auto& component = *internal_add_component_generic(MaterialComponentType::Reflection, nullptr);
    auto& layer     = component.layer(0);
    auto& _data2    = layer.data2();
    layer.setMask(mask);
    layer.setCubemap(cubemap);
    layer.setData2(mixFactor, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentRefraction(const std::string& cubemapName, const std::string& maskFile, float refractiveIndex, float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap = Engine::priv::MaterialLoader::LoadTextureCubemap(cubemapName);
    Texture* mask    = Engine::priv::MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap) {
        cubemap = Engine::Resources::getCurrentScene()->skybox()->texture();
    }
    auto& component = *internal_add_component_generic(MaterialComponentType::Refraction, nullptr);
    auto& layer     = component.layer(0);
    auto& _data2    = layer.data2();
    layer.setMask(mask);
    layer.setCubemap(cubemap);
    layer.setData2(mixFactor, refractiveIndex, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentParallaxOcclusion(const std::string& textureFile, float heightScale){
    Texture* texture = Engine::priv::MaterialLoader::LoadTextureNormal(textureFile);
    auto& component  = *internal_add_component_generic(MaterialComponentType::ParallaxOcclusion, texture);
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
    Material::setF0Color(f0Color.color.r, f0Color.color.g, f0Color.color.b);
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
    const auto& t = MATERIAL_PROPERTIES[(unsigned int)materialPhysics];
    setF0Color(std::get<0>(t), std::get<1>(t), std::get<2>(t));
    setSmoothness(std::get<3>(t));
    setMetalness(std::get<4>(t));
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
    for (size_t i = 0; i < m_Components.size(); ++i) {
        auto* component = m_Components[i];
        if (!component) {
            break; //TODO: this assumes there will never be a null component before defined components. be careful here. Improve this logic to avoid the issue?
        }
        component->update(dt);
    }
}
#pragma endregion