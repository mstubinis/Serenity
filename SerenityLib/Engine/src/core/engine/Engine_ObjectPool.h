#pragma once
#ifndef ENGINE_OBJECT_POOL_H
#define ENGINE_OBJECT_POOL_H

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/utils/Utils.h>

namespace Engine{
namespace epriv{
    template<typename R> struct HandleEntry final{
        uint nextFreeIndex : 12; //
        uint version : 15;       // 29 bits
        uint active : 1;         //  total
        uint endOfList : 1;      //
        R* resource;
        HandleEntry(){
            version = 1;
            nextFreeIndex = 1;
            active = endOfList = 0;
            resource = nullptr;
        }
        explicit HandleEntry(uint _nextFreeIndex){
            nextFreeIndex = ++_nextFreeIndex;
            version = 1;
            active = endOfList = 0; 
            resource = nullptr;
        }
    };

    template<typename T> class ObjectPool final{
        private:
            uint              MAX_ENTRIES;
            uint              m_activeEntryCount;
            uint              m_firstFreeEntry;
            HandleEntry<T>*   m_Pool;
        public:
            ObjectPool(uint numEntries){ 
                MAX_ENTRIES = numEntries; 
                m_Pool = new HandleEntry<T>[MAX_ENTRIES]; 
                reset(); 
            }
            ~ObjectPool(){ destruct(); }

            void destruct(){
                for(uint i = 0; i < MAX_ENTRIES; ++i){
                    SAFE_DELETE(m_Pool[i].resource);
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
                return c;
            } 
            void reset(){
                m_activeEntryCount = 0;
                m_firstFreeEntry = 1;
                for (uint i = 0; i < MAX_ENTRIES - 1; ++i){
                    m_Pool[i] = HandleEntry<T>(i + 1);
                }
                m_Pool[MAX_ENTRIES - 1] = HandleEntry<T>();
                m_Pool[MAX_ENTRIES - 1].endOfList = true;
            }
            void update(Handle& _handle,T* _ptr){
                const uint index = _handle.index - 1;
                if(m_Pool[index].version == _handle.version && m_Pool[index].active){
                    m_Pool[index].resource = _ptr;
                }
            }
            void update(const uint& _id,T* _ptr){
                const uint index = _id - 1;
                if(m_Pool[index].active){
                    m_Pool[index].resource = _ptr;
                }
            }
            Handle add(T* _ptr,uint _type){
                const uint newIndex = m_firstFreeEntry - 1;
                if(newIndex >= MAX_ENTRIES) return Handle(); //null handle
                m_firstFreeEntry = m_Pool[newIndex].nextFreeIndex;
                m_Pool[newIndex].nextFreeIndex = 0;
                ++m_Pool[newIndex].version;
                if (m_Pool[newIndex].version == 0){
                    m_Pool[newIndex].version = 1;
                }
                m_Pool[newIndex].active = true;
                m_Pool[newIndex].resource = _ptr;
                ++m_activeEntryCount;
                return Handle(newIndex+1, m_Pool[newIndex].version, _type);
            }
            uint add(T* _ptr){
                const uint newIndex = m_firstFreeEntry - 1;
                if(newIndex >= MAX_ENTRIES) return 0; //null entity
                m_firstFreeEntry = m_Pool[newIndex].nextFreeIndex;
                m_Pool[newIndex].nextFreeIndex = 0;
                ++m_Pool[newIndex].version;
                if (m_Pool[newIndex].version == 0){
                    m_Pool[newIndex].version = 1;
                }
                m_Pool[newIndex].active = true;
                m_Pool[newIndex].resource = _ptr;
                ++m_activeEntryCount;
                return newIndex+1;
            }
            void remove(uint _index){
                const uint index = _index - 1;
                if(m_Pool[index].active){
                    m_Pool[index].nextFreeIndex = m_firstFreeEntry;
                    m_Pool[index].active = false;
                    SAFE_DELETE(m_Pool[index].resource);
                    m_firstFreeEntry = _index;
                    --m_activeEntryCount;		
                }
            }
            void remove(Handle& _handle){
                const uint index = _handle.index;
                const uint realIndex = index - 1;
                if(m_Pool[realIndex].version == _handle.version && m_Pool[realIndex].active){
                    m_Pool[realIndex].nextFreeIndex = m_firstFreeEntry;
                    m_Pool[realIndex].active = false;
                    SAFE_DELETE(m_Pool[realIndex].resource);
                    m_firstFreeEntry = index;
                    --m_activeEntryCount;		
                }
            }
            T* get(Handle& _handle){
                T* outPtr = nullptr;
                if (!get(_handle, outPtr)) return nullptr;
                return outPtr;
            }
            T* get(const uint& _id){
                T* outPtr = nullptr;
                if (!get(_id, outPtr)) return nullptr;
                return outPtr;
            }
            bool get(const Handle& _handle, T*& _outPtr){
                const uint index = _handle.index - 1;
                if (m_Pool[index].version != _handle.version || !m_Pool[index].active){
                    _outPtr = nullptr;
                    return false;
                }
                _outPtr = m_Pool[index].resource;
                return true;
            }
            bool get(const uint& _index, T*& _outPtr){
                const uint index = _index - 1;
                if(!m_Pool[index].resource){
                    _outPtr = nullptr;
                    return false;
                }
                _outPtr = m_Pool[index].resource;
                return true;
            }
            template<typename U> inline bool getAs(Handle& _handle, U*& _outPtr){
                T* _void = nullptr;
                const bool rv = get(_handle,_void);
                _outPtr = (U*)_void; //use union_cast ? was in the original source
                return rv;
            }
            template<typename U> inline bool getAs(const uint& _index, U*& _outPtr){
                T* _void = nullptr;
                const bool rv = get(_index,_void);
                _outPtr = (U*)_void; //use union_cast ? was in the original source
                return rv;
            }
            template<typename U> inline void getAsFast(const uint& _index, U*& _outPtr){
                const uint index = _index - 1;
                if(!m_Pool[index].resource){
                    _outPtr = nullptr;
                    return;
                }
                _outPtr = (U*)m_Pool[index].resource;
            }
            template<typename U> inline void getAsFast(Handle& _handle, U*& _outPtr){
                const uint index = _handle.index - 1;
                if(!m_Pool[index].resource){
                    _outPtr = nullptr;
                    return;
                }
                _outPtr = (U*)m_Pool[index].resource;
            }
            template<typename U> inline U* getAsFast(Handle& _handle){
                const uint index = _handle.index - 1;
                if(!m_Pool[index].resource) return nullptr;
                return (U*)m_Pool[index].resource;
            }
            template<typename U> inline U* getAsFast(const uint& _index){
                const uint index = _index - 1;
                if(!m_Pool[index].resource) return nullptr;
                return (U*)m_Pool[index].resource;
            }
    };
};
};


#endif