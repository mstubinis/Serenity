#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/mesh/MeshLoading.h>
#include <core/engine/mesh/MeshCollisionFactory.h>

#include <core/engine/Engine.h>
#include <core/engine/resources/Engine_Resources.h>

#include <boost/filesystem.hpp>

using namespace Engine;
using namespace std;

MeshRequest::MeshRequest() {
    fileOrData    = "";
    fileExtension = "";
    fileExists    = false;
    async         = false;
    threshold     = 0.0005f;
}
MeshRequest::MeshRequest(const string& _filenameOrData, float _threshold):MeshRequest() {
    fileOrData = _filenameOrData;
    threshold = _threshold;
    if (fileOrData != "") {
        fileExtension = boost::filesystem::extension(fileOrData);
        if (boost::filesystem::exists(fileOrData)) {
            fileExists = true;
        }
    }
}
MeshRequest::~MeshRequest() {

}
void MeshRequest::request() {
    async = false;
    epriv::InternalMeshRequestPublicInterface::Request(*this);
}
void MeshRequest::requestAsync() {
    async = true;
    epriv::InternalMeshRequestPublicInterface::Request(*this);
}

void epriv::InternalMeshRequestPublicInterface::Request(MeshRequest& meshRequest) {
    if (meshRequest.fileOrData != "") {
        if (meshRequest.fileExists) {
            bool valid = epriv::InternalMeshRequestPublicInterface::Populate(meshRequest);
            if (valid){
                if (meshRequest.async){
                    const auto& reference = std::ref(meshRequest);
                    const auto& job = boost::bind(&epriv::InternalMeshRequestPublicInterface::LoadCPU, reference);
                    const auto& cbk = boost::bind(&epriv::InternalMeshRequestPublicInterface::LoadGPU, reference);
                    epriv::threading::addJobWithPostCallback(job, cbk);
                }else{
                    epriv::InternalMeshRequestPublicInterface::LoadCPU(meshRequest);
                    epriv::InternalMeshRequestPublicInterface::LoadGPU(meshRequest);
                }
            }else{
                //some wierd error happened
            }
        }else{
            //we got either an invalid file or memory data
        }
    }
}
bool epriv::InternalMeshRequestPublicInterface::Populate(MeshRequest& meshRequest) {
    if (meshRequest.fileExtension != ".objcc") {
        meshRequest.importer.scene = const_cast<aiScene*>(meshRequest.importer.importer.ReadFile(meshRequest.fileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace));
        meshRequest.importer.root = meshRequest.importer.scene->mRootNode;

        auto& scene = *meshRequest.importer.scene;
        auto& root = *meshRequest.importer.root;

        if (!&scene || scene.mFlags & AI_SCENE_FLAGS_INCOMPLETE || !&root) {
            return false;
        }
        epriv::MeshLoader::LoadPopulateGlobalNodes(root, meshRequest.map);
        epriv::MeshLoader::LoadProcessNodeNames(meshRequest.fileOrData,meshRequest.parts, scene, root, meshRequest.map);
    }else{
        MeshRequestPart part;
        part.name = meshRequest.fileOrData;
        part.mesh = new Mesh();
        part.mesh->setName(part.name);
        part.handle = epriv::Core::m_Engine->m_ResourceManager.m_Resources->add(part.mesh, ResourceType::Mesh);
        meshRequest.parts.push_back(part);
    }
    return true;
}
void epriv::InternalMeshRequestPublicInterface::LoadCPU(MeshRequest& meshRequest) {
    if (meshRequest.fileExtension != ".objcc") {
        auto& root = *meshRequest.importer.root;
        auto& scene = *meshRequest.importer.scene;
        uint count = 0;
        epriv::MeshLoader::LoadProcessNodeData(meshRequest.parts, scene, root, meshRequest.map, count);
    }else{
        VertexData* vertexData = epriv::MeshLoader::LoadFrom_OBJCC(meshRequest.fileOrData);
        auto& partMesh = *(meshRequest.parts[0].mesh);
        //cpu
        partMesh.m_VertexData = vertexData;
        partMesh.m_threshold = meshRequest.threshold;

        partMesh.calculate_radius();
        partMesh.m_CollisionFactory = new epriv::MeshCollisionFactory(partMesh);
    }
}
void epriv::InternalMeshRequestPublicInterface::LoadGPU(MeshRequest& meshRequest) {
    for (auto& part : meshRequest.parts) {
        if (part.mesh) {
            part.mesh->load_gpu();
            part.mesh->EngineResource::load();
        }
    }
    if(meshRequest.async)
        delete(&meshRequest); //yes its ugly, but its needed. see Resources::loadMeshAsync()
}