#pragma once
#ifndef ENGINE_MODEL_INSTANCE_HANDLE_H
#define ENGINE_MODEL_INSTANCE_HANDLE_H
 
class ComponentModel;
class ModelInstance;

class ModelInstanceHandle final {
    private:
        ComponentModel* m_ComponentModel = nullptr;
        size_t          m_Index = 0;
    public:
        ModelInstanceHandle() = default;
        ModelInstanceHandle(size_t Index, ComponentModel& componentModel)
            : m_ComponentModel{ &componentModel }
            , m_Index{ Index }
        {}
        operator ModelInstance& () noexcept;
        ModelInstance& get() noexcept;

        bool operator==(ModelInstanceHandle& other) noexcept;
        bool operator==(const ModelInstanceHandle& other) noexcept;
        bool operator!=(ModelInstanceHandle& other) noexcept;
        bool operator!=(const ModelInstanceHandle& other) noexcept;
};

#endif