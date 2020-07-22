#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/mesh/Skeleton.h>
#include <core/engine/mesh/AnimationIncludes.h>
#include <core/engine/mesh/MeshImportedData.h>
#include <core/engine/math/Engine_Math.h>

using namespace Engine::priv;
using namespace std;

MeshSkeleton::MeshSkeleton() {
    clear();
}
MeshSkeleton::~MeshSkeleton() {
}
unsigned int MeshSkeleton::numBones() const {
    return m_NumBones; 
}
void MeshSkeleton::clear() {
    m_NumBones = 0;
    m_BoneMapping.clear();
}