#pragma once
#ifndef ENGINE_OBJECT_POOL_H
#define ENGINE_OBJECT_POOL_H

#include <memory>
#include <cstdint>

#include "Engine_ResourceHandle.h"
#include "Engine_Utils.h"

typedef unsigned int uint;
typedef std::uint32_t uint32;

namespace Engine{
    namespace epriv{
        template<typename R>
        struct HandleEntry final{
            uint32 nextFreeIndex : 12;
            uint32 counter : 15;
            uint32 active : 1;
            uint32 endOfList : 1;
            R* resource;
            HandleEntry(){
                counter = 1; nextFreeIndex, active, endOfList = 0; resource = nullptr;
            }
            explicit HandleEntry(uint32 _nextFreeIndex){
                nextFreeIndex = _nextFreeIndex; counter = 1; active, endOfList = 0; resource = nullptr;
            }
        };
        #pragma region ObjectPool
        template<typename T>
        class ObjectPool final: private Engine::epriv::noncopyable{
            private:
                uint32 MAX_ENTRIES;
                HandleEntry<T>* m_Pool;
                uint32 m_activeEntryCount;
                uint32 m_firstFreeEntry;
            public:
                ObjectPool(uint numEntries){ 
                    MAX_ENTRIES = numEntries; m_Pool = new HandleEntry<T>[MAX_ENTRIES]; reset(); 
                }
                ~ObjectPool(){ destruct(); }

                void destruct(){
                    for(uint i = 0; i < MAX_ENTRIES; ++i){
                        if(m_Pool[i].resource){
                            SAFE_DELETE(m_Pool[i].resource);
                        }
                    }
                    delete[] m_Pool;
                }
                uint maxEntries(){
                    return MAX_ENTRIES;
                }
                uint size(){
                    uint c = 0; uint i;
                    for(i = 0; i < MAX_ENTRIES; ++i){
                        if(m_Pool[i].resource){ c = i; }
                    }
                    return i;
                }
                void reset(){
                    m_activeEntryCount = 0;
                    m_firstFreeEntry = 0;
                    for (uint i = 0; i < MAX_ENTRIES - 1; ++i){
                        m_Pool[i] = HandleEntry<T>(i + 1);
                    }
                    m_Pool[MAX_ENTRIES - 1] = HandleEntry<T>();
                    m_Pool[MAX_ENTRIES - 1].endOfList = true;
                }
                void update(Handle& handle,T* ptr){
                    const uint index = handle.index;
                    if(m_Pool[index].counter == handle.counter && m_Pool[index].active){
                        m_Pool[index].resource = ptr;
                    }
                }
                void update(const uint& id,T* ptr){
                    if(m_Pool[id].active){
                        m_Pool[id].resource = ptr;
                    }
                }
                Handle add(T* ptr,uint type){
                    const uint newIndex = m_firstFreeEntry;
                    if(newIndex >= MAX_ENTRIES) return Handle(); //null handle
                    m_firstFreeEntry = m_Pool[newIndex].nextFreeIndex;
                    m_Pool[newIndex].nextFreeIndex = 0;
                    ++m_Pool[newIndex].counter;
                    if (m_Pool[newIndex].counter == 0){
                        m_Pool[newIndex].counter = 1;
                    }
                    m_Pool[newIndex].active = true;
                    m_Pool[newIndex].resource = ptr;
                    ++m_activeEntryCount;
                    return Handle(newIndex, m_Pool[newIndex].counter, type);
                }
                uint add(T* ptr){
                    const uint newIndex = m_firstFreeEntry;
                    if(newIndex >= MAX_ENTRIES) return uint(-1); //null entity
                    m_firstFreeEntry = m_Pool[newIndex].nextFreeIndex;
                    m_Pool[newIndex].nextFreeIndex = 0;
                    ++m_Pool[newIndex].counter;
                    if (m_Pool[newIndex].counter == 0){
                        m_Pool[newIndex].counter = 1;
                    }
                    m_Pool[newIndex].active = true;
                    m_Pool[newIndex].resource = ptr;
                    ++m_activeEntryCount;
                    return newIndex;
                }
                void remove(uint index){
                    if(m_Pool[index].active){
                        m_Pool[index].nextFreeIndex = m_firstFreeEntry;
                        m_Pool[index].active = false;
                        if(m_Pool[index].resource){
                            SAFE_DELETE(m_Pool[index].resource);
                        }
                        m_firstFreeEntry = index;
                        --m_activeEntryCount;		
                    }
                }
                void remove(Handle& handle){
                    const uint32 index = handle.index;
                    if(m_Pool[index].counter == handle.counter && m_Pool[index].active){
                        m_Pool[index].nextFreeIndex = m_firstFreeEntry;
                        m_Pool[index].active = false;
                        SAFE_DELETE(m_Pool[index].resource);
                        m_firstFreeEntry = index;
                        --m_activeEntryCount;		
                    }
                }

                T* get(Handle& handle){
                    T* outPtr = nullptr;
                    if (!get(handle, outPtr)) return nullptr;
                    return outPtr;
                }
                T* get(const uint& id){
                    T* outPtr = nullptr;
                    if (!get(id, outPtr)) return nullptr;
                    return outPtr;
                }
                bool get(const Handle& handle, T*& outPtr){
                    const uint index = handle.index;
                    if (m_Pool[index].counter != handle.counter || !m_Pool[index].active){
                        outPtr = nullptr;
                        return false;
                    }
                    outPtr = m_Pool[index].resource;
                    return true;
                }
                bool get(const uint& index, T*& outPtr){
                    if(!m_Pool[index].resource){
                        outPtr = nullptr;
                        return false;
                    }
                    outPtr = m_Pool[index].resource;
                    return true;
                }
                template<typename U> inline bool getAs(Handle& handle, U*& outPtr){
                    T* _void = nullptr;
                    const bool rv = get(handle,_void);
                    outPtr = (U*)_void; //use union_cast ? was in the original source
                    return rv;
                }
                template<typename U> inline bool getAs(const uint& index, U*& outPtr){
                    T* _void = nullptr;
                    const bool rv = get(index,_void);
                    outPtr = (U*)_void; //use union_cast ? was in the original source
                    return rv;
                }
                template<typename U> inline void getAsFast(const uint& index, U*& outPtr){
                    if(!m_Pool[index].resource){
                        outPtr = nullptr;
                        return;
                    }
                    outPtr = (U*)m_Pool[index].resource;
                }
                template<typename U> inline void getAsFast(Handle& handle, U*& outPtr){
                    if(!m_Pool[handle.index].resource){
                        outPtr = nullptr;
                        return;
                    }
                    outPtr = (U*)m_Pool[handle.index].resource;
                }
                template<typename U> inline U* getAsFast(Handle& handle){
                    if(!m_Pool[handle.index].resource) return nullptr;
                    return (U*)m_Pool[handle.index].resource;
                }
                template<typename U> inline U* getAsFast(const uint& index){
                    if(!m_Pool[index].resource) return nullptr;
                    return (U*)m_Pool[index].resource;
                }
        };
        #pragma endregion
    };
};


#endif