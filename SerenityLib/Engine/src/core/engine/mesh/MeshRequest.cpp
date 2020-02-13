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
    importer_ptr.reset(NEW Assimp::Importer{});
    scene    = nullptr;
    root     = nullptr;
}
Engine::priv::AssimpSceneImport::~AssimpSceneImport() {

}
Engine::priv::AssimpSceneImport::AssimpSceneImport(const AssimpSceneImport& other) {
    importer_ptr = other.importer_ptr;
    scene        = other.scene;
    root         = other.root;
}
Engine::priv::AssimpSceneImport& Engine::priv::AssimpSceneImport::operator=(const AssimpSceneImport& other) {
    if (&other != this) {
        importer_ptr = other.importer_ptr;
        scene        = other.scene;
        root         = other.root;
    }
    return *this;
}


MeshRequest::MeshRequestPart::MeshRequestPart() {
    mesh   = nullptr;
    name   = "";
    handle = Handle();
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





MeshRequest::MeshRequest(const string& filenameOrData, float threshold){
    fileExtension = "";
    fileExists    = false;
    async         = false;
    fileOrData    = filenameOrData;
    threshold     = threshold;
    if (!fileOrData.empty()) {
        fileExtension = boost::filesystem::extension(fileOrData);
        if (boost::filesystem::exists(fileOrData)) {
            fileExists = true;
        }
    }
}
MeshRequest::~MeshRequest() {
    SAFE_DELETE_MAP(map);
}

MeshRequest::MeshRequest(const MeshRequest& other) {
    fileOrData    = other.fileOrData;
    fileExtension = other.fileExtension;
    fileExists    = other.fileExists;
    async         = other.async;
    threshold     = other.threshold;
    importer      = other.importer;
    map           = other.map;
    parts         = other.parts;
}
MeshRequest& MeshRequest::operator=(const MeshRequest& other) {
    if (&other != this) {
        fileOrData    = other.fileOrData;
        fileExtension = other.fileExtension;
        fileExists    = other.fileExists;
        async         = other.async;
        threshold     = other.threshold;
        importer      = other.importer;
        map           = other.map;
        parts         = other.parts;
    }
    return *this;
}
MeshRequest::MeshRequest(MeshRequest&& other) noexcept {
    fileOrData    = std::move(other.fileOrData);
    fileExtension = std::move(other.fileExtension);
    fileExists    = std::move(other.fileExists);
    async         = std::move(other.async);
    threshold     = std::move(other.threshold);
    map           = std::move(other.map);
    parts         = std::move(other.parts);
    importer      = (other.importer);
}
MeshRequest& MeshRequest::operator=(MeshRequest&& other) noexcept {
    if (&other != this) {
        fileOrData    = std::move(other.fileOrData);
        fileExtension = std::move(other.fileExtension);
        fileExists    = std::move(other.fileExists);
        async         = std::move(other.async);
        threshold     = std::move(other.threshold);
        map           = std::move(other.map);
        parts         = std::move(other.parts);
        importer      = (other.importer);
    }
    return *this;
}



void MeshRequest::request() {
    async = false;
    InternalMeshRequestPublicInterface::Request(*this);
}
void MeshRequest::requestAsync() {
    async = true;
    InternalMeshRequestPublicInterface::Request(*this);
}

void InternalMeshRequestPublicInterface::Request(MeshRequest& meshRequest) {
    if (!meshRequest.fileOrData.empty()) {
        if (meshRequest.fileExists) {
            const bool valid = InternalMeshRequestPublicInterface::Populate(meshRequest);
            if (valid){
                if (meshRequest.async){
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
    if (meshRequest.fileExtension != ".objcc") {
        meshRequest.importer.scene = const_cast<aiScene*>(meshRequest.importer.importer_ptr->ReadFile(meshRequest.fileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace));
        meshRequest.importer.root  = meshRequest.importer.scene->mRootNode;

        auto& scene = *meshRequest.importer.scene;
        auto& root  = *meshRequest.importer.root;

        if (!&scene || (scene.mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !&root) {
            return false;
        }
        MeshLoader::LoadPopulateGlobalNodes(root, meshRequest.map);
        MeshLoader::LoadProcessNodeNames(meshRequest.fileOrData, meshRequest.parts, scene, root, meshRequest.map);
    }else{
        MeshRequest::MeshRequestPart part;
        part.name   = meshRequest.fileOrData;
        part.mesh   = NEW Mesh();
        part.mesh->setName(part.name);
        part.handle = Core::m_Engine->m_ResourceManager.m_Resources.add(part.mesh, ResourceType::Mesh);
        meshRequest.parts.push_back(std::move(part));
    }
    return true;
}
void InternalMeshRequestPublicInterface::LoadCPU(MeshRequest& meshRequest) {
    if (meshRequest.fileExtension != ".objcc") {
        auto& root   = *meshRequest.importer.root;
        auto& scene  = *meshRequest.importer.scene;
        uint count   = 0;
        MeshLoader::LoadProcessNodeData(meshRequest.parts, scene, root, meshRequest.map, count);
        MeshLoader::SaveTo_OBJCC(*const_cast<VertexData*>(meshRequest.parts[0].mesh->m_VertexData), meshRequest.fileOrData + ".objcc");
    }else{ //objcc
        VertexData* vertexData   = MeshLoader::LoadFrom_OBJCC(meshRequest.fileOrData);
        Mesh& mesh               = *meshRequest.parts[0].mesh;
        mesh.m_VertexData        = vertexData;
        mesh.m_threshold         = meshRequest.threshold;
        InternalMeshPublicInterface::CalculateRadius(mesh);
        SAFE_DELETE(mesh.m_CollisionFactory);
        mesh.m_CollisionFactory  = NEW MeshCollisionFactory(mesh);
    }
}
void InternalMeshRequestPublicInterface::LoadGPU(MeshRequest& meshRequest) {
    for (auto& part : meshRequest.parts) {
        InternalMeshPublicInterface::LoadGPU(*part.mesh);
        part.mesh->EngineResource::load();
    }
}