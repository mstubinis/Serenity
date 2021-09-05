#pragma once
#ifndef ENGINE_MODEL_INSTANCE_HANDLE_H
#define ENGINE_MODEL_INSTANCE_HANDLE_H
 
class ComponentModel;
class ModelInstance;

class ModelInstanceHandle final {
    private:
        size_t            m_Index          = 0;
        ComponentModel*   m_ComponentModel = nullptr;
    public:
        ModelInstanceHandle() = default;
        ModelInstanceHandle(size_t Index, ComponentModel& componentModel)
            : m_Index{ Index }
            , m_ComponentModel{ &componentModel }
        {}
        operator ModelInstance&() noexcept;
        [[nodiscard]] ModelInstance& get() noexcept;

        bool operator==(ModelInstanceHandle&) noexcept;
        bool operator==(const ModelInstanceHandle&) noexcept;
        bool operator!=(ModelInstanceHandle&) noexcept;
        bool operator!=(const ModelInstanceHandle&) noexcept;
};

#endif