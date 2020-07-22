#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/MaterialRequest.h>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/system/Engine.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace Engine;
using namespace Engine::priv;


MaterialRequestPart::MaterialRequestPart() {
}
MaterialRequestPart::~MaterialRequestPart() {

}
MaterialRequestPart::MaterialRequestPart(const MaterialRequestPart& other) {
    m_Material = other.m_Material;
    m_Name     = other.m_Name;
    m_Handle   = other.m_Handle;
    m_TextureRequests = other.m_TextureRequests;
}
MaterialRequestPart& MaterialRequestPart::operator=(const MaterialRequestPart& other) {
    if (&other != this) {
        m_Material = other.m_Material;
        m_Name     = other.m_Name;
        m_Handle   = other.m_Handle;
        m_TextureRequests = other.m_TextureRequests;
    }
    return *this;
}



MaterialRequest::MaterialRequest(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular, const string& ao, const string& metalness, const string& smoothness){ 
    m_Part.m_Name = name;
    m_Part.m_TextureRequests.emplace_back(  NEW TextureRequest( diffuse, false, ImageInternalFormat::SRGB8_ALPHA8 )  );
    m_Part.m_TextureRequests.emplace_back(  NEW TextureRequest( normal, false, ImageInternalFormat::RGBA8 )  );
    m_Part.m_TextureRequests.emplace_back(  NEW TextureRequest( glow, false, ImageInternalFormat::R8 )  );
    m_Part.m_TextureRequests.emplace_back(  NEW TextureRequest( specular, false, ImageInternalFormat::R8 )  );
    m_Part.m_TextureRequests.emplace_back(  NEW TextureRequest( ao, false, ImageInternalFormat::R8 )  );
    m_Part.m_TextureRequests.emplace_back(  NEW TextureRequest( metalness, false, ImageInternalFormat::R8 )  );
    m_Part.m_TextureRequests.emplace_back(  NEW TextureRequest( smoothness, false, ImageInternalFormat::R8 )  );
}
MaterialRequest::MaterialRequest(const string& name, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness) {
    m_Part.m_Name     = name;
    m_Part.m_Material = NEW Material(name, diffuse, normal, glow, specular, ao, metalness, smoothness);
    m_Part.m_Handle   = Core::m_Engine->m_ResourceManager.m_Resources.add(m_Part.m_Material, ResourceType::Material);
}

MaterialRequest::~MaterialRequest() {

}
MaterialRequest::MaterialRequest(const MaterialRequest& other) {
    m_Async = other.m_Async;
    m_Part  = other.m_Part;
}
MaterialRequest& MaterialRequest::operator=(const MaterialRequest& other) {
    if (&other != this) {
        m_Async = other.m_Async;
        m_Part  = other.m_Part;
    }
    return *this;
}

void MaterialRequest::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        m_Async = true;
        for (auto& textureRequest : m_Part.m_TextureRequests) {
            textureRequest->part.async = true;
        }
    }else{
        m_Async = false;
    }
    InternalMaterialRequestPublicInterface::Request(*this);
}

void InternalMaterialRequestPublicInterface::Request(MaterialRequest& request) {
    request.m_Part.m_Material = NEW Material();
    request.m_Part.m_Material->setName(request.m_Part.m_Name);
    request.m_Part.m_Handle = Core::m_Engine->m_ResourceManager.m_Resources.add(request.m_Part.m_Material, ResourceType::Material);

    auto size = request.m_Part.m_TextureRequests.size();
    for (size_t i = 0; i < size; ++i) {
        if (request.m_Part.m_TextureRequests[i]->fileExists) {
            auto& component = request.m_Part.m_Material->addComponent(static_cast<MaterialComponentType::Type>(i), "DEFAULT");
        }
    }

    if (request.m_Async) {
        for (auto& textureRequest : request.m_Part.m_TextureRequests) {
            textureRequest->request(true);
        }
        auto lambda_cpu = [=]() {
            InternalMaterialRequestPublicInterface::LoadCPU(const_cast<MaterialRequest&>(request));
        };
        auto lambda_gpu = [=]() {
            InternalMaterialRequestPublicInterface::LoadGPU(const_cast<MaterialRequest&>(request));
        };
        threading::addJobWithPostCallback(lambda_cpu, lambda_gpu);
    }else{
        for (auto& textureRequest : request.m_Part.m_TextureRequests) {
            textureRequest->request();
        }
        InternalMaterialRequestPublicInterface::LoadCPU(request);
        InternalMaterialRequestPublicInterface::LoadGPU(request);
    }
}
void InternalMaterialRequestPublicInterface::LoadCPU(MaterialRequest& request) {
    InternalMaterialPublicInterface::LoadCPU(*request.m_Part.m_Material);
}
void InternalMaterialRequestPublicInterface::LoadGPU(MaterialRequest& request) {
    const auto& texture_requests = request.m_Part.m_TextureRequests;
    unsigned int count = 0;
    for (size_t i = 0; i < texture_requests.size(); ++i) {
        if (texture_requests[i]->fileExists) {
            request.m_Part.m_Material->getComponent(count).layer(0).setTexture(texture_requests[i]->file);
            ++count;
        }
    }
    InternalMaterialPublicInterface::LoadGPU(*request.m_Part.m_Material);
}