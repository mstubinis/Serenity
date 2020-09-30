#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/model/ModelInstanceHandle.h>
#include <ecs/ComponentModel.h>

ModelInstanceHandle::operator ModelInstance& () noexcept { 
    return m_ComponentModel->getModel(m_Index); 
}
ModelInstance& ModelInstanceHandle::get() noexcept { 
    return m_ComponentModel->getModel(m_Index); 
}

bool ModelInstanceHandle::operator==(ModelInstanceHandle& other) noexcept {
    return (m_Index == other.m_Index && m_ComponentModel->getOwner() == other.m_ComponentModel->getOwner());
}
bool ModelInstanceHandle::operator==(const ModelInstanceHandle& other) noexcept {
    return (m_Index == other.m_Index && m_ComponentModel->getOwner() == other.m_ComponentModel->getOwner());
}
bool ModelInstanceHandle::operator!=(ModelInstanceHandle& other) noexcept {
    return !operator==(other);
}
bool ModelInstanceHandle::operator!=(const ModelInstanceHandle& other) noexcept {
    return !operator==(other);
}