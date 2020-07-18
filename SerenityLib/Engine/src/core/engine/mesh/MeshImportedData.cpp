#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

priv::MeshImportedData::MeshImportedData() {
}
priv::MeshImportedData::~MeshImportedData() {
    vector_clear(file_points);
    vector_clear(file_uvs);
    vector_clear(file_normals);
    vector_clear(points);
    vector_clear(uvs);
    vector_clear(normals);
    vector_clear(binormals);
    vector_clear(tangents);
    vector_clear(indices);
    m_Bones.clear();
}
