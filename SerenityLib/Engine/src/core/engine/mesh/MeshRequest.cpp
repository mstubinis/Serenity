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
            bool valid = populate();
            if (valid) {
                auto l_cpu = [meshRequest{ *this }]() mutable {
                    InternalMeshRequestPublicInterface::LoadCPU(meshRequest);
                };
                auto l_gpu = [meshRequest{ *this }]() mutable {
                    auto mutex = meshRequest.m_Parts[0].handle.getMutex();
                    if (mutex) {
                        std::lock_guard lock(*mutex);
                        for (auto& part : meshRequest.m_Parts) {
                            auto& mesh = *part.handle.get<Mesh>();
                            InternalMeshPublicInterface::LoadGPU(mesh);
                        }
                    }
                    meshRequest.m_Callback();
                };
                if (m_Async || !Engine::priv::threading::isMainThread()) {
                    if (Engine::priv::threading::isMainThread()) {
                        threading::addJobWithPostCallback(std::move(l_cpu), std::move(l_gpu), 1U);
                    }else{
                        threading::submitTaskForMainThread([c{ std::move(l_cpu) }, g{ std::move(l_gpu) }]() mutable {
                            threading::addJobWithPostCallback(std::move(c), std::move(g), 1U);
                        });
                    }
                }else{
                    l_cpu();
                    l_gpu();
                }
            }else{
                ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): mesh request - " << m_FileOrData << " was invalid (populate() failed)!")
            }
        }
    }
}

bool MeshRequest::populate() {
    if (m_FileExtension != ".objcc" && m_FileExtension != ".smsh") {
        m_Importer.m_AIScene = const_cast<aiScene*>(m_Importer.m_Importer_ptr->ReadFile(m_FileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace));
        m_Importer.m_AIRoot  = m_Importer.m_AIScene->mRootNode;

        auto& scene { *m_Importer.m_AIScene };
        auto& root  { *m_Importer.m_AIRoot };

        if (!&scene || (scene.mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !&root) {
            return false;
        }
        auto* rootNode{ NEW Engine::priv::MeshInfoNode{ root.mName.C_Str(), Engine::Math::assimpToGLMMat4(root.mTransformation) } };
        MeshLoader::LoadPopulateGlobalNodes(scene, rootNode, nullptr, rootNode, &root, *this);
    }else{
        auto& part   = m_Parts.emplace_back();
        part.name    = m_FileOrData;
        part.handle  = Engine::Resources::addResource<Mesh>();
        part.handle.get<Mesh>()->setName(m_FileOrData);
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
