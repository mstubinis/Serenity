
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
    : m_CollisionLoadingFlags{ std::move(other.m_CollisionLoadingFlags) }
    , m_AssimpImporter       { (other.m_AssimpImporter) }
    , m_Callback             { std::move(other.m_Callback) }
    , m_Parts                { std::move(other.m_Parts) }
    , m_FileOrData           { std::move(other.m_FileOrData) }
    , m_FileExtension        { std::move(other.m_FileExtension) }
    , m_Threshold            { std::move(other.m_Threshold) }
    , m_FileExists           { std::move(other.m_FileExists) }
    , m_Async                { std::move(other.m_Async) }
{}
MeshRequest& MeshRequest::operator=(MeshRequest&& other) noexcept {
    if (this != &other) {
        m_CollisionLoadingFlags = std::move(other.m_CollisionLoadingFlags);
        m_AssimpImporter        = (other.m_AssimpImporter);
        m_Callback              = std::move(other.m_Callback);
        m_Parts                 = std::move(other.m_Parts);
        m_FileOrData            = std::move(other.m_FileOrData);
        m_FileExtension         = std::move(other.m_FileExtension);
        m_Threshold             = std::move(other.m_Threshold);
        m_FileExists            = std::move(other.m_FileExists);
        m_Async                 = std::move(other.m_Async);
    }
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
                    for (auto& part : meshRequest.m_Parts) {
                        auto& mesh = *part.handle.get<Mesh>();
                        PublicMesh::LoadGPU(mesh);
                    }
                    auto handles = Engine::create_and_reserve<std::vector<Handle>>(meshRequest.m_Parts.size());
                    for (auto& part : meshRequest.m_Parts) {
                        handles.emplace_back(part.handle);
                    }
                    meshRequest.m_Callback(handles);
                };
                if (m_Async || !Engine::priv::threading::isMainThread()) {
                    Engine::priv::threading::addJobWithPostCallback(std::move(l_cpu), std::move(l_gpu));
                } else {
                    l_cpu();
                    l_gpu();
                }
            } else {
                ENGINE_LOG(__FUNCTION__ << "(): mesh request - " << m_FileOrData << " was invalid (populate() failed)!")
            }
        }
    }
}

bool MeshRequest::populate() {
    if (m_FileExtension != ".smsh") {
        m_AssimpImporter.m_AIScene = const_cast<aiScene*>(m_AssimpImporter.m_Importer_ptr->ReadFile(m_FileOrData, aiProcess_Triangulate /* | aiProcess_FlipUVs*/ | aiProcess_CalcTangentSpace));
        double unitScaleFactor;
        m_AssimpImporter.m_AIScene->mMetaData->Get("UnitScaleFactor", unitScaleFactor);
        //ENGINE_LOG(m_FileOrData << ": UnitScaleFactor: " << unitScaleFactor);
        //m_AssimpImporter.m_AIScene->mMetaData->Set(0, "UnitScaleFactor", 1.0);
        auto aiscene              = m_AssimpImporter.m_AIScene;
        if (!aiscene || (aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !aiscene->mRootNode) {
            return false;
        }
        MeshLoader::LoadPopulateGlobalNodes(*m_AssimpImporter.m_AIScene,  *this);
    } else {
        auto& part         = m_Parts.emplace_back();
        part.name          = m_FileOrData;
        part.handle        = Engine::Resources::addResource<Mesh>();
        part.handle.get<Mesh>()->setName(m_FileOrData);
        part.fileOrData    = m_FileOrData;
        part.fileExtension = m_FileExtension;
        part.fileExists    = m_FileExists;
    }
    return true;
}
void PublicMeshRequest::LoadCPU(MeshRequest& meshRequest) {
    if (meshRequest.m_FileExtension != ".smsh") {
        auto& aiscene     = *meshRequest.m_AssimpImporter.m_AIScene;
        if (Engine::priv::threading::isWorkerThreadStopped()) {
            return;
        }
        MeshLoader::LoadProcessNodeData(meshRequest, aiscene, *aiscene.mRootNode);
        if (Engine::priv::threading::isWorkerThreadStopped()) {
            return;
        }

        for (auto& part : meshRequest.m_Parts) {
            {
                //std::unique_lock lock{ part.handle };
                auto& mesh = *part.handle.get<Mesh>();
                mesh.setName(part.name);
                mesh.m_CPUData = std::move(part.cpuData);
                mesh.m_CPUData.m_NodesData = std::move(part.nodesData);

                //TODO: this just saves any imported model as the engine's optimized format. Remove this upon release.
                std::string saveFileName = part.fileOrData.substr(0, part.fileOrData.find_last_of(".")) + ".smsh";
                SMSH_File::SaveFile(saveFileName.c_str(), mesh.m_CPUData);
            }
        }
    } else if (meshRequest.m_FileExtension == ".smsh") {
        for (auto& part : meshRequest.m_Parts) {
            if (Engine::priv::threading::isWorkerThreadStopped()) {
                return;
            }
            part.cpuData.m_Threshold = meshRequest.m_Threshold;
            SMSH_File::LoadFile(part.fileOrData.c_str(), part.cpuData);
            if (Engine::priv::threading::isWorkerThreadStopped()) {
                return;
            }
            part.cpuData.internal_calculate_radius();
            part.cpuData.m_CollisionFactory = NEW MeshCollisionFactory{ part.cpuData, meshRequest.m_CollisionLoadingFlags };
            {
                //std::unique_lock lock{ part.handle };
                auto& mesh = *part.handle.get<Mesh>();
                mesh.setName(part.name);
                mesh.m_CPUData = std::move(part.cpuData);
            }
        }
    }
}
