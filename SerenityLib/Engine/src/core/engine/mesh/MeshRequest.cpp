#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshCollisionFactory.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/mesh/smsh.h>
#include <core/engine/system/window/Window.h>

#include <core/engine/system/Engine.h>

using namespace Engine;
using namespace Engine::priv;

Engine::priv::AssimpSceneImport::AssimpSceneImport() {
    m_Importer_ptr = std::make_shared<Assimp::Importer>();
}

MeshRequest::MeshRequest(const std::string& filenameOrData, float threshold, MeshCollisionLoadingFlag::Flag flags, std::function<void()>&& callback)
    : m_FileOrData            { filenameOrData }
    , m_Threshold             { threshold }
    , m_Callback              { std::move(callback) }
    , m_CollisionLoadingFlags { flags }
{
    if (!m_FileOrData.empty()) {
        m_FileExtension = std::filesystem::path(m_FileOrData).extension().string();
        if (std::filesystem::exists(m_FileOrData)) {
            m_FileExists = true;
        }
    }
}
MeshRequest::MeshRequest(MeshRequest&& other) noexcept
    : m_FileOrData    { std::move(other.m_FileOrData) }
    , m_FileExtension { std::move(other.m_FileExtension) }
    , m_FileExists    { std::move(other.m_FileExists) }
    , m_Async         { std::move(other.m_Async) }
    , m_Threshold     { std::move(other.m_Threshold) }
    , m_MeshNodeMap   { std::move(other.m_MeshNodeMap) }
    , m_Parts         { std::move(other.m_Parts) }
    , m_Callback      { std::move(other.m_Callback) }
    , m_Importer      { other.m_Importer }
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
    m_Async = (inAsync && Engine::hardware_concurrency() > 1);

    if (!m_FileOrData.empty()) {
        if (m_FileExists) {
            bool valid = InternalMeshRequestPublicInterface::Populate(*this);
            if (valid) {
                MeshRequest copyRequest{ *this };
                auto lambda_cpu = [copyRequest]() mutable {
                    InternalMeshRequestPublicInterface::LoadCPU(copyRequest);
                };
                auto lambda_gpu = [copyRequest]() mutable {
                    InternalMeshRequestPublicInterface::LoadGPU(copyRequest);
                    copyRequest.m_Callback();
                };

                if (m_Async || !Engine::priv::threading::isMainThread()) {
                    threading::addJobWithPostCallback(lambda_cpu, lambda_gpu);
                }else{
                    lambda_cpu();
                    lambda_gpu();
                }
            }
        }
    }
}

bool InternalMeshRequestPublicInterface::Populate(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".objcc" && meshRequest.m_FileExtension != ".smsh") {
        meshRequest.m_Importer.m_AIScene = const_cast<aiScene*>(meshRequest.m_Importer.m_Importer_ptr->ReadFile(meshRequest.m_FileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace));
        meshRequest.m_Importer.m_AIRoot  = meshRequest.m_Importer.m_AIScene->mRootNode;

        auto& scene { *meshRequest.m_Importer.m_AIScene };
        auto& root  { *meshRequest.m_Importer.m_AIRoot };

        if (!&scene || (scene.mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !&root) {
            return false;
        }
        auto* rootNode{ NEW Engine::priv::MeshInfoNode{ root.mName.C_Str(), Engine::Math::assimpToGLMMat4(root.mTransformation) } };
        MeshLoader::LoadPopulateGlobalNodes(scene, rootNode, nullptr, rootNode, &root, meshRequest);
    }else{
        auto& part   = meshRequest.m_Parts.emplace_back();
        part.name    = meshRequest.m_FileOrData;
        part.handle  = Engine::Resources::addResource<Mesh>();
        part.handle.get<Mesh>()->setName(part.name);
    }
    return true;
}
void InternalMeshRequestPublicInterface::LoadCPU(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".objcc" && meshRequest.m_FileExtension != ".smsh") {
        auto& scene{ *meshRequest.m_Importer.m_AIScene };
        auto& root{ *meshRequest.m_Importer.m_AIRoot };
        unsigned int count{ 0 };
        MeshLoader::LoadProcessNodeData(meshRequest, scene, root, count);
        const char* saveFileName = (meshRequest.m_FileOrData.substr(0, meshRequest.m_FileOrData.find_last_of(".")) + ".smsh").c_str();
        auto& part = meshRequest.m_Parts[0];
        SMSH_File::SaveFile(saveFileName, part.cpuData);
        auto mutex = part.handle.getMutex();
        if (mutex) {
            std::lock_guard lock(*mutex);
            auto& mesh = *part.handle.get<Mesh>();
            mesh.setName(part.name);
            mesh.m_CPUData = std::move(part.cpuData);
        }
    }else if (meshRequest.m_FileExtension == ".smsh") {
        for (auto& part : meshRequest.m_Parts) {
            SMSH_File::LoadFile(meshRequest.m_FileOrData.c_str(), part.cpuData);
            part.cpuData.m_Threshold = meshRequest.m_Threshold;
            part.cpuData.internal_calculate_radius();
            part.cpuData.m_CollisionFactory = (NEW MeshCollisionFactory(part.cpuData, meshRequest.m_CollisionLoadingFlags));
            auto mutex = part.handle.getMutex();
            if (mutex) {
                std::lock_guard lock(*mutex);
                auto& mesh = *part.handle.get<Mesh>();
                mesh.setName(part.name);
                mesh.m_CPUData = std::move(part.cpuData);
            }
        }
    }else{ //objcc
        for (auto& part : meshRequest.m_Parts) {
            part.cpuData.m_VertexData = (MeshLoader::LoadFrom_OBJCC(meshRequest.m_FileOrData));
            part.cpuData.m_Threshold  = meshRequest.m_Threshold;
            part.cpuData.internal_calculate_radius();
            part.cpuData.m_CollisionFactory = (NEW MeshCollisionFactory(part.cpuData, meshRequest.m_CollisionLoadingFlags));
            auto mutex = part.handle.getMutex();
            if (mutex) {
                std::lock_guard lock(*mutex);
                auto& mesh = *part.handle.get<Mesh>();
                mesh.setName(part.name);
                mesh.m_CPUData = std::move(part.cpuData);
            }
        }
    }
}
void InternalMeshRequestPublicInterface::LoadGPU(MeshRequest& meshRequest) {
    auto mutex = meshRequest.m_Parts[0].handle.getMutex();
    if (mutex) {
        std::lock_guard lock(*mutex);
        for (auto& part : meshRequest.m_Parts) {
            InternalMeshPublicInterface::LoadGPU(*part.handle.get<Mesh>());
            part.handle.get<Mesh>()->Resource::load();
        }
    }
}