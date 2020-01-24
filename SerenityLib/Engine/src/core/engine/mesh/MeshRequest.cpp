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
    importer_ptr = other.importer_ptr;
    scene        = other.scene;
    root         = other.root;
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
    auto& other_ = const_cast<MeshRequest::MeshRequestPart&>(other);
    mesh         = other_.mesh;
    name         = other_.name;
    handle       = other_.handle;
}
MeshRequest::MeshRequestPart& MeshRequest::MeshRequestPart::operator=(const MeshRequest::MeshRequestPart& other) {
    auto& other_ = const_cast<MeshRequest::MeshRequestPart&>(other);
    mesh         = other_.mesh;
    name         = other_.name;
    handle       = other_.handle;
    return *this;
}
MeshRequest::MeshRequestPart::MeshRequestPart(MeshRequest::MeshRequestPart&& other) noexcept {
    using std::swap;
    std::swap(mesh, other.mesh);
    std::swap(name, other.name);
    std::swap(handle, other.handle);
}
MeshRequest::MeshRequestPart& MeshRequest::MeshRequestPart::operator=(MeshRequest::MeshRequestPart&& other) noexcept {
    using std::swap;
    std::swap(mesh, other.mesh);
    std::swap(name, other.name);
    std::swap(handle, other.handle);
    return *this;
}





MeshRequest::MeshRequest(const string& _filenameOrData, float _threshold){
    fileExtension = "";
    fileExists    = false;
    async         = false;
    fileOrData    = _filenameOrData;
    threshold     = _threshold;
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

    parts.reserve(other.parts.size());
    for (auto& part : other.parts) {
        parts.emplace_back(part);
    }
}
MeshRequest& MeshRequest::operator=(const MeshRequest& other) {
    fileOrData    = other.fileOrData;
    fileExtension = other.fileExtension;
    fileExists    = other.fileExists;
    async         = other.async;
    threshold     = other.threshold;
    importer      = other.importer;

    map           = other.map;

    parts.reserve(other.parts.size());
    for (auto& part : other.parts) {
        parts.emplace_back(part);
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
                    const auto& job = [=]() { 
                        InternalMeshRequestPublicInterface::LoadCPU(const_cast<MeshRequest&>(meshRequest)); 
                    };
                    const auto& cbk = [=]() { 
                        InternalMeshRequestPublicInterface::LoadGPU(const_cast<MeshRequest&>(meshRequest));
                    };
                    threading::addJobWithPostCallback(job, cbk);
                }else{
                    InternalMeshRequestPublicInterface::LoadCPU(meshRequest);
                    InternalMeshRequestPublicInterface::LoadGPU(meshRequest);
                }
            }else{
                //some wierd error happened
            }
        }else{
            //we got either an invalid file or memory data
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
        part.name = meshRequest.fileOrData;
        part.mesh = NEW Mesh();
        part.mesh->setName(part.name);
        part.handle = Core::m_Engine->m_ResourceManager.m_Resources->add(part.mesh, ResourceType::Mesh);
        meshRequest.parts.push_back(part);
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
        mesh.m_CollisionFactory  = NEW MeshCollisionFactory(mesh);
    }
}
void InternalMeshRequestPublicInterface::LoadGPU(MeshRequest& meshRequest) {
    for (auto& part : meshRequest.parts) {
        if (part.mesh) {
            InternalMeshPublicInterface::LoadGPU(*part.mesh);
            part.mesh->EngineResource::load();
        }
    }
}