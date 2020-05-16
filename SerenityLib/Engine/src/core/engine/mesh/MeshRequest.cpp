#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshCollisionFactory.h>

#include <core/engine/system/Engine.h>

#include <boost/filesystem.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

Engine::priv::AssimpSceneImport::AssimpSceneImport() {
    m_Importer_ptr.reset(NEW Assimp::Importer{});
}
Engine::priv::AssimpSceneImport::~AssimpSceneImport() {

}
Engine::priv::AssimpSceneImport::AssimpSceneImport(const AssimpSceneImport& other) {
    m_Importer_ptr   = other.m_Importer_ptr;
    m_AIScene        = other.m_AIScene;
    m_AIRoot         = other.m_AIRoot;
}
Engine::priv::AssimpSceneImport& Engine::priv::AssimpSceneImport::operator=(const AssimpSceneImport& other) {
    if (&other != this) {
        m_Importer_ptr = other.m_Importer_ptr;
        m_AIScene      = other.m_AIScene;
        m_AIRoot       = other.m_AIRoot;
    }
    return *this;
}


MeshRequest::MeshRequestPart::MeshRequestPart() {
}
MeshRequest::MeshRequestPart::~MeshRequestPart() {

}
MeshRequest::MeshRequestPart::MeshRequestPart(const MeshRequest::MeshRequestPart& other) {
    mesh         = other.mesh;
    name         = other.name;
    handle       = other.handle;
}
MeshRequest::MeshRequestPart& MeshRequest::MeshRequestPart::operator=(const MeshRequest::MeshRequestPart& other) {
    if (&other != this) {
        mesh         = other.mesh;
        name         = other.name;
        handle       = other.handle;
    }
    return *this;
}
MeshRequest::MeshRequestPart::MeshRequestPart(MeshRequest::MeshRequestPart&& other) noexcept {
    mesh   = std::exchange(other.mesh, nullptr);
    name   = std::move(other.name);
    handle = std::move(other.handle);
}
MeshRequest::MeshRequestPart& MeshRequest::MeshRequestPart::operator=(MeshRequest::MeshRequestPart&& other) noexcept {
    if (&other != this) {
        mesh   = std::exchange(other.mesh, nullptr);
        name   = std::move(other.name);
        handle = std::move(other.handle);
    }
    return *this;
}





MeshRequest::MeshRequest(const string& filenameOrData, const float threshold){
    m_FileOrData    = filenameOrData;
    m_Threshold     = threshold;
    if (!m_FileOrData.empty()) {
        m_FileExtension = boost::filesystem::extension(m_FileOrData);
        if (boost::filesystem::exists(m_FileOrData)) {
            m_FileExists = true;
        }
    }
}
MeshRequest::~MeshRequest() {
    //TODO: this is causing problems
    //SAFE_DELETE_MAP(m_Map);
}
MeshRequest::MeshRequest(const MeshRequest& other) {
    m_FileOrData     = other.m_FileOrData;
    m_FileExtension  = other.m_FileExtension;
    m_FileExists     = other.m_FileExists;
    m_Async          = other.m_Async;
    m_Threshold      = other.m_Threshold;
    m_Importer       = other.m_Importer;
    m_Map            = other.m_Map;
    m_Parts          = other.m_Parts;
}
MeshRequest& MeshRequest::operator=(const MeshRequest& other) {
    if (&other != this) {
        m_FileOrData    = other.m_FileOrData;
        m_FileExtension = other.m_FileExtension;
        m_FileExists    = other.m_FileExists;
        m_Async         = other.m_Async;
        m_Threshold     = other.m_Threshold;
        m_Importer      = other.m_Importer;
        m_Map           = other.m_Map;
        m_Parts         = other.m_Parts;
    }
    return *this;
}
MeshRequest::MeshRequest(MeshRequest&& other) noexcept {
    m_FileOrData    = std::move(other.m_FileOrData);
    m_FileExtension = std::move(other.m_FileExtension);
    m_FileExists    = std::move(other.m_FileExists);
    m_Async         = std::move(other.m_Async);
    m_Threshold     = std::move(other.m_Threshold);
    m_Map           = std::move(other.m_Map);
    m_Parts         = std::move(other.m_Parts);
    m_Importer      = (other.m_Importer);
}
MeshRequest& MeshRequest::operator=(MeshRequest&& other) noexcept {
    if (&other != this) {
        m_FileOrData    = std::move(other.m_FileOrData);
        m_FileExtension = std::move(other.m_FileExtension);
        m_FileExists    = std::move(other.m_FileExists);
        m_Async         = std::move(other.m_Async);
        m_Threshold     = std::move(other.m_Threshold);
        m_Map           = std::move(other.m_Map);
        m_Parts         = std::move(other.m_Parts);
        m_Importer      = (other.m_Importer);
    }
    return *this;
}
void MeshRequest::request() {
    m_Async = false;
    InternalMeshRequestPublicInterface::Request(*this);
}
void MeshRequest::requestAsync() {
    if (Engine::priv::threading::hardware_concurrency() > 1) {
        m_Async = true;
        InternalMeshRequestPublicInterface::Request(*this);
    }else{
        MeshRequest::request();
    }
}

void InternalMeshRequestPublicInterface::Request(MeshRequest& meshRequest) {
    if (!meshRequest.m_FileOrData.empty()) {
        if (meshRequest.m_FileExists) {
            const bool valid = InternalMeshRequestPublicInterface::Populate(meshRequest);
            if (valid){
                if (meshRequest.m_Async){
                    const auto& job = [meshRequest]() {
                        InternalMeshRequestPublicInterface::LoadCPU(const_cast<MeshRequest&>(meshRequest)); 
                    };
                    const auto& cbk = [meshRequest]() {
                        InternalMeshRequestPublicInterface::LoadGPU(const_cast<MeshRequest&>(meshRequest));
                    };
                    threading::addJobWithPostCallback(job, cbk);
                }else{
                    InternalMeshRequestPublicInterface::LoadCPU(meshRequest);
                    InternalMeshRequestPublicInterface::LoadGPU(meshRequest);
                }
            }
        }
    }
}
bool InternalMeshRequestPublicInterface::Populate(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".objcc") {
        meshRequest.m_Importer.m_AIScene = const_cast<aiScene*>(meshRequest.m_Importer.m_Importer_ptr->ReadFile(meshRequest.m_FileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace));
        meshRequest.m_Importer.m_AIRoot  = meshRequest.m_Importer.m_AIScene->mRootNode;

        auto& scene = *meshRequest.m_Importer.m_AIScene;
        auto& root  = *meshRequest.m_Importer.m_AIRoot;

        if (!&scene || (scene.mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !&root) {
            return false;
        }
        MeshLoader::LoadPopulateGlobalNodes(root, meshRequest.m_Map);
        MeshLoader::LoadProcessNodeNames(meshRequest.m_FileOrData, meshRequest.m_Parts, scene, root, meshRequest.m_Map);
    }else{
        MeshRequest::MeshRequestPart part;
        part.name   = meshRequest.m_FileOrData;
        part.mesh   = NEW Mesh();
        part.mesh->setName(part.name);
        part.handle = Core::m_Engine->m_ResourceManager.m_Resources.add(part.mesh, ResourceType::Mesh);
        meshRequest.m_Parts.push_back(std::move(part));
    }
    return true;
}
void InternalMeshRequestPublicInterface::LoadCPU(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".objcc") {
        auto& scene  = *meshRequest.m_Importer.m_AIScene;
        auto& root   = *meshRequest.m_Importer.m_AIRoot;
        uint count   = 0;
        MeshLoader::LoadProcessNodeData(meshRequest.m_Parts, scene, root, meshRequest.m_Map, count);
        MeshLoader::SaveTo_OBJCC(*const_cast<VertexData*>(meshRequest.m_Parts[0].mesh->m_VertexData), meshRequest.m_FileOrData + ".objcc");
    }else{ //objcc
        VertexData* vertexData   = MeshLoader::LoadFrom_OBJCC(meshRequest.m_FileOrData);
        Mesh& mesh               = *meshRequest.m_Parts[0].mesh;
        mesh.m_VertexData        = vertexData;
        mesh.m_Threshold         = meshRequest.m_Threshold;
        InternalMeshPublicInterface::CalculateRadius(mesh);
        SAFE_DELETE(mesh.m_CollisionFactory);
        mesh.m_CollisionFactory  = NEW MeshCollisionFactory(mesh);
    }
}
void InternalMeshRequestPublicInterface::LoadGPU(MeshRequest& meshRequest) {
    for (auto& part : meshRequest.m_Parts) {
        InternalMeshPublicInterface::LoadGPU(*part.mesh);
        part.mesh->EngineResource::load();
    }
}