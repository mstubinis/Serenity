#include <core/engine/mesh/MeshRequest.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/MeshImpl.h>
#include <core/engine/mesh/MeshLoading.h>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <boost/filesystem.hpp>

using namespace Engine;
using namespace std;

MeshRequest::MeshRequest() {
    fileOrData = "";
    threshold  = 0.0005f;
}
MeshRequest::MeshRequest(const string& _filenameOrData, float _threshold) {
    fileOrData = _filenameOrData;
    threshold  = _threshold;
}
MeshRequest::~MeshRequest() {

}
void MeshRequest::request() {
    if (fileOrData != "") {
        //first determine if the file is data or a file path
        if (boost::filesystem::exists(fileOrData)) {
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(fileOrData, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                return;
            }
            unordered_map<string, epriv::BoneNode*> map;
            epriv::MeshLoader::LoadPopulateGlobalNodes(*scene->mRootNode,map);

            vector<MeshRequestPart> impls;
            epriv::MeshLoader::LoadProcessNode(impls, *scene, *scene->mRootNode, map);

        }else{
            //we got either an invalid file or memory data
        }
    }
}