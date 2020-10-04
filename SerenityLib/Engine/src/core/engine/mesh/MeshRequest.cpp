#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/smsh.h>
#include <core/engine/system/window/Window.h>

#include <core/engine/system/Engine.h>

#include <boost/filesystem.hpp>

using namespace Engine;
using namespace Engine::priv;

Engine::priv::AssimpSceneImport::AssimpSceneImport() {
    m_Importer_ptr.reset(NEW Assimp::Importer{});
}

MeshRequest::MeshRequest(const std::string& filenameOrData, const float threshold, std::function<void()>&& callback) 
    : m_FileOrData{ filenameOrData }
    , m_Threshold{ threshold }
    , m_Callback{ std::move(callback) }
{
    if (!m_FileOrData.empty()) {
        m_FileExtension = boost::filesystem::extension(m_FileOrData);
        if (boost::filesystem::exists(m_FileOrData)) {
            m_FileExists = true;
        }
    }
}
MeshRequest::MeshRequest(MeshRequest&& other) noexcept
    : m_FileOrData{ std::move(other.m_FileOrData) }
    , m_FileExtension{ std::move(other.m_FileExtension) }
    , m_FileExists{ std::move(other.m_FileExists) }
    , m_Async{ std::move(other.m_Async) }
    , m_Threshold{ std::move(other.m_Threshold) }
    , m_MeshNodeMap{ std::move(other.m_MeshNodeMap) }
    , m_Parts{ std::move(other.m_Parts) }
    , m_Callback{ std::move(other.m_Callback) }
    , m_Importer{ other.m_Importer }
{}
MeshRequest& MeshRequest::operator=(MeshRequest&& other) noexcept {
    m_FileOrData    = std::move(other.m_FileOrData);
    m_FileExtension = std::move(other.m_FileExtension);
    m_FileExists    = std::move(other.m_FileExists);
    m_Async         = std::move(other.m_Async);
    m_Threshold     = std::move(other.m_Threshold);
    m_MeshNodeMap   = std::move(other.m_MeshNodeMap);
    m_Parts         = std::move(other.m_Parts);
    m_Callback      = std::move(other.m_Callback);
    m_Importer      = (other.m_Importer);
    return *this;
}
void MeshRequest::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        m_Async = true;
    }else{
        m_Async = false;
    }
    InternalMeshRequestPublicInterface::Request(*this);
}

void InternalMeshRequestPublicInterface::Request(MeshRequest& meshRequest) {
    if (!meshRequest.m_FileOrData.empty()) {
        if (meshRequest.m_FileExists) {
            bool valid = InternalMeshRequestPublicInterface::Populate(meshRequest);
            if (valid){
                if (meshRequest.m_Async || std::this_thread::get_id() != Resources::getWindow().getOpenglThreadID()){
                    threading::addJobWithPostCallback(
                        [meshRequest]() mutable {
                            InternalMeshRequestPublicInterface::LoadCPU(meshRequest);
                        },
                        [meshRequest]() mutable {
                            InternalMeshRequestPublicInterface::LoadGPU(meshRequest);
                            meshRequest.m_Callback();
                        }
                    );
                }else{
                    InternalMeshRequestPublicInterface::LoadCPU(meshRequest);
                    InternalMeshRequestPublicInterface::LoadGPU(meshRequest);
                    meshRequest.m_Callback();
                }
            }
        }
    }
}
bool InternalMeshRequestPublicInterface::Populate(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".objcc" && meshRequest.m_FileExtension != ".smsh") {
        meshRequest.m_Importer.m_AIScene = const_cast<aiScene*>(meshRequest.m_Importer.m_Importer_ptr->ReadFile(meshRequest.m_FileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace));
        meshRequest.m_Importer.m_AIRoot  = meshRequest.m_Importer.m_AIScene->mRootNode;

        auto& scene{ *meshRequest.m_Importer.m_AIScene };
        auto& root{ *meshRequest.m_Importer.m_AIRoot };

        if (!&scene || (scene.mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !&root) {
            return false;
        }
        auto* rootNode{ NEW Engine::priv::MeshInfoNode{ root.mName.C_Str(), Engine::Math::assimpToGLMMat4(root.mTransformation) } };
        MeshLoader::LoadPopulateGlobalNodes(scene, rootNode, nullptr, rootNode, &root, meshRequest);
    }else{
        MeshRequestPart part{};
        part.name    = meshRequest.m_FileOrData;
        part.handle  = Core::m_Engine->m_ResourceManager.m_ResourceModule.emplace<Mesh>();
        Mesh* mesh   = part.handle.get<Mesh>();
        mesh->setName(part.name);
        meshRequest.m_Parts.push_back(std::move(part));
    }
    return true;
}
void InternalMeshRequestPublicInterface::LoadCPU(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".objcc" && meshRequest.m_FileExtension != ".smsh") {
        auto& scene{ *meshRequest.m_Importer.m_AIScene };
        auto& root{ *meshRequest.m_Importer.m_AIRoot };
        unsigned int count{ 0 };
        MeshLoader::LoadProcessNodeData(meshRequest, scene, root, count);
        SMSH_File::SaveFile((meshRequest.m_FileOrData.substr(0, meshRequest.m_FileOrData.find_last_of(".")) + ".smsh").c_str(), *meshRequest.m_Parts[0].handle.get<Mesh>());
    }else if (meshRequest.m_FileExtension == ".smsh") {
        Mesh& mesh{ *meshRequest.m_Parts[0].handle.get<Mesh>() };
        SMSH_File::LoadFile(&mesh, meshRequest.m_FileOrData.c_str());
        mesh.m_Threshold = meshRequest.m_Threshold;
        InternalMeshPublicInterface::CalculateRadius(mesh);
        SAFE_DELETE(mesh.m_CollisionFactory);
        mesh.m_CollisionFactory = NEW MeshCollisionFactory(mesh);
    }else{ //objcc
        VertexData* vertexData{ MeshLoader::LoadFrom_OBJCC(meshRequest.m_FileOrData) };
        Mesh& mesh{ *meshRequest.m_Parts[0].handle.get<Mesh>() };
        mesh.m_VertexData        = vertexData;
        mesh.m_Threshold         = meshRequest.m_Threshold;
        InternalMeshPublicInterface::CalculateRadius(mesh);
        SAFE_DELETE(mesh.m_CollisionFactory);
        mesh.m_CollisionFactory  = NEW MeshCollisionFactory(mesh);
    }
}
void InternalMeshRequestPublicInterface::LoadGPU(MeshRequest& meshRequest) {
    for (auto& part : meshRequest.m_Parts) {
        InternalMeshPublicInterface::LoadGPU(*part.handle.get<Mesh>());
        part.handle.get<Mesh>()->Resource::load();
    }
}