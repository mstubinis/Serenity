
#include <serenity/resources/mesh/MeshRequest.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/MeshLoading.h>
#include <serenity/resources/mesh/MeshCollisionFactory.h>
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/mesh/smsh.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/Engine.h>

#include <filesystem>

using namespace Engine;
using namespace Engine::priv;

Engine::priv::AssimpSceneImport::AssimpSceneImport() {
    m_Importer_ptr = std::make_shared<Assimp::Importer>();
}
MeshRequest::MeshRequest(std::string_view filenameOrData, float threshold, MeshCollisionLoadingFlag::Flag flags, MeshRequestCallback&& callback)
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
    , m_NodeData { std::move(other.m_NodeData) }
    , m_NodeStrVector { std::move(other.m_NodeStrVector) }
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
    m_NodeData  = std::move(other.m_NodeData);
    m_NodeStrVector = std::move(other.m_NodeStrVector);
    m_Parts         = std::move(other.m_Parts);
    m_Callback      = std::move(other.m_Callback);
    m_Importer      = (other.m_Importer);
    return *this;
}
void MeshRequest::request(bool inAsync) {
    m_Async = (inAsync && Engine::hardware_concurrency() > 1);

    if (!m_FileOrData.empty()) {
        if (m_FileExists) {
            auto info = Resources::getResource<Mesh>(m_FileOrData);
            if (info.m_Resource) {
                //mesh already loaded
                return;
            }

            bool valid = populate();
            if (valid) {
                auto l_cpu = [meshRequest{ *this }]() mutable {
                    PublicMeshRequest::LoadCPU(meshRequest);
                };
                auto l_gpu = [meshRequest{ *this }]() mutable {
                    auto mutex = meshRequest.m_Parts[0].handle.getMutex();
                    if (mutex) {
                        std::unique_lock lock{ *mutex };
                        for (auto& part : meshRequest.m_Parts) {
                            auto& mesh = *part.handle.get<Mesh>();
                            PublicMesh::LoadGPU(mesh);
                        }
                    }
                    auto handles = Engine::create_and_reserve<std::vector<Handle>>(meshRequest.m_Parts.size());
                    for (auto& part : meshRequest.m_Parts) {
                        handles.emplace_back(part.handle);
                    }
                    meshRequest.m_Callback(handles);
                };
                if (m_Async || !Engine::priv::threading::isMainThread()) {
                    if (Engine::priv::threading::isMainThread()) {
                        threading::addJobWithPostCallback(l_cpu, l_gpu, 1U);
                    }else{
                        threading::submitTaskForMainThread([=]() mutable {
                            threading::addJobWithPostCallback(l_cpu, l_gpu, 1U);
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
        auto& aiscene        = *m_Importer.m_AIScene;
        auto& aiRootNode     = *m_Importer.m_AIRoot;
        if (!&aiscene || (aiscene.mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !&aiRootNode) {
            return false;
        }

        MeshLoader::LoadPopulateGlobalNodes(aiscene, &aiRootNode, *this);
    }else{
        auto& part   = m_Parts.emplace_back();
        part.name    = m_FileOrData;
        part.handle  = Engine::Resources::addResource<Mesh>();
        part.handle.get<Mesh>()->setName(m_FileOrData);
    }
    return true;
}
void PublicMeshRequest::LoadCPU(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".objcc" && meshRequest.m_FileExtension != ".smsh") {
        auto& aiscene     = *meshRequest.m_Importer.m_AIScene;
        auto& aiRootNode  = *meshRequest.m_Importer.m_AIRoot;
        MeshLoader::LoadProcessNodeData(meshRequest, aiscene, aiRootNode);
        auto& part        = meshRequest.m_Parts[0];

        //TODO: this just saves any imported model as the engine's optimized format. Remove this upon release.
        std::string saveFileName = (meshRequest.m_FileOrData.substr(0, meshRequest.m_FileOrData.find_last_of(".")) + ".smsh").c_str();
        SMSH_File::SaveFile(saveFileName.c_str(), part.cpuData);


        auto mutex = part.handle.getMutex();
        if (mutex) {
            std::unique_lock lock{ *mutex };
            auto& mesh = *part.handle.get<Mesh>();
            mesh.setName(part.name);
            mesh.m_CPUData = std::move(part.cpuData);
            mesh.m_CPUData.m_NodeData = std::move(meshRequest.m_NodeData);
        }
    }else if (meshRequest.m_FileExtension == ".smsh") {
        for (auto& part : meshRequest.m_Parts) {
            SMSH_File::LoadFile(meshRequest.m_FileOrData.c_str(), part.cpuData);
            part.cpuData.m_Threshold        = meshRequest.m_Threshold;
            part.cpuData.internal_calculate_radius();
            part.cpuData.m_CollisionFactory = (NEW MeshCollisionFactory(part.cpuData, meshRequest.m_CollisionLoadingFlags));
            auto mutex                      = part.handle.getMutex();
            if (mutex) {
                std::unique_lock lock{ *mutex };
                auto& mesh = *part.handle.get<Mesh>();
                mesh.setName(part.name);
                mesh.m_CPUData = std::move(part.cpuData);
                mesh.m_CPUData.m_NodeData = std::move(meshRequest.m_NodeData);
            }
        }
    }else{ //objcc
        for (auto& part : meshRequest.m_Parts) {
            part.cpuData.m_VertexData       = (MeshLoader::LoadFrom_OBJCC(meshRequest.m_FileOrData));
            part.cpuData.m_Threshold        = meshRequest.m_Threshold;
            part.cpuData.internal_calculate_radius();
            part.cpuData.m_CollisionFactory = (NEW MeshCollisionFactory(part.cpuData, meshRequest.m_CollisionLoadingFlags));
            auto mutex                      = part.handle.getMutex();
            if (mutex) {
                std::unique_lock lock{ *mutex };
                auto& mesh = *part.handle.get<Mesh>();
                mesh.setName(part.name);
                mesh.m_CPUData = std::move(part.cpuData);
                mesh.m_CPUData.m_NodeData = std::move(meshRequest.m_NodeData);
            }
        }
    }
}
