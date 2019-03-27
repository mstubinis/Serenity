#include <core/engine/mesh/MeshImpl.h>
#include <core/engine/mesh/VertexData.h>
#include <core/engine/Engine_Utils.h>
#include <core/engine/mesh/MeshCollisionFactory.h>

#include <iostream>

using namespace Engine;
using namespace std;

epriv::MeshImpl::MeshImpl() {

}
epriv::MeshImpl::~MeshImpl() {

}

void epriv::MeshImpl::unload_cpu() {
    SAFE_DELETE(m_Skeleton);
    SAFE_DELETE(m_CollisionFactory);
    cout << "(Mesh) ";
}
void epriv::MeshImpl::unload_gpu() {
    SAFE_DELETE(m_VertexData);
}

void epriv::MeshImpl::load_cpu() {
    /*
    if (m_File != "") {
        _loadFromFile(super, m_File, m_threshold);
    }
    _calculateMeshRadius(*m_VertexData);
    m_CollisionFactory = new epriv::MeshCollisionFactory(super, *m_VertexData);
    */
}
void epriv::MeshImpl::load_gpu() {
    m_VertexData->finalize(); //transfer vertex data to gpu
    cout << "(Mesh) ";
}