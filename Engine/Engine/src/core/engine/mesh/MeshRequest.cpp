#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/MeshLoading.h>

#include <core/engine/Engine.h>
#include <core/engine/resources/Engine_Resources.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/filesystem.hpp>

using namespace Engine;
using namespace std;

MeshRequest::MeshRequest() {
    fileOrData = "";
    async = false;
    threshold  = 0.0005f;
}
MeshRequest::MeshRequest(const string& _filenameOrData, float _threshold) {
    fileOrData = _filenameOrData;
    async = false;
    threshold  = _threshold;
}
MeshRequest::~MeshRequest() {

}


void _request(MeshRequest& meshRequest,bool async) {
    if (meshRequest.fileOrData != "") {
        //first determine if the file is data or a file path
        if (boost::filesystem::exists(meshRequest.fileOrData)) {
            const string& extension = boost::filesystem::extension(meshRequest.fileOrData);
            if (extension != ".objcc") {
                Assimp::Importer importer;
                const aiScene* scene = importer.ReadFile(meshRequest.fileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
                const auto& root = *scene->mRootNode;
                if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !&root) {
                    return;
                }
                unordered_map<string, epriv::BoneNode*> map;
                epriv::MeshLoader::LoadPopulateGlobalNodes(root, map);
                epriv::MeshLoader::LoadProcessNode(meshRequest.parts, *scene, root, map);
                for (auto& part : meshRequest.parts) {
                    if (part.mesh) {
                        part.handle = epriv::Core::m_Engine->m_ResourceManager.m_Resources->add(part.mesh, ResourceType::Mesh);
                    }
                }
            }else{
                VertexData* vertexData = epriv::MeshLoader::LoadFrom_OBJCC(meshRequest.fileOrData);
                MeshRequestPart part;
                part.name = meshRequest.fileOrData;
                part.mesh = new Mesh(vertexData, part.name, async);
                part.handle = epriv::Core::m_Engine->m_ResourceManager.m_Resources->add(part.mesh, ResourceType::Mesh);
                meshRequest.parts.push_back(part);
            }
            if (async) {
                for (const auto& part : meshRequest.parts) {
                    const auto& reference = std::ref(*part.mesh);
                    const auto& job = boost::bind(&epriv::InternalMeshPublicInterface::LoadCPU, reference);
                    const auto& cbk = boost::bind(&epriv::InternalMeshPublicInterface::LoadGPU, reference);
                    epriv::threading::addJobWithPostCallback(job, cbk);
                }
            }
        }else{
            //we got either an invalid file or memory data
        }
    }
}


void MeshRequest::request() {
    _request(*this, false);
}
void MeshRequest::requestAsync() {
    async = true;
    _request(*this, true);
}