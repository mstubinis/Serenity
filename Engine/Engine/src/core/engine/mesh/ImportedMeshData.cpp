#include "core/engine/mesh/ImportedMeshData.h"

#include <core/engine/Engine_Utils.h>

using namespace Engine;
using namespace std;

epriv::ImportedMeshData::ImportedMeshData() {
}
epriv::ImportedMeshData::~ImportedMeshData() {
    clear(); 
}
void epriv::ImportedMeshData::clear() {
    vector_clear(file_points);
    vector_clear(file_uvs);
    vector_clear(file_normals);
    vector_clear(file_triangles);
    vector_clear(points);
    vector_clear(uvs);
    vector_clear(normals);
    vector_clear(binormals);
    vector_clear(tangents);
    vector_clear(indices);
}
