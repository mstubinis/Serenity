#pragma once
#ifndef ENGINE_OBJECT_POOL_H
#define ENGINE_OBJECT_POOL_H

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/resources/Handle.h>

namespace Engine{
    namespace epriv{
        template<typename R> struct HandleEntry final{
            std::uint32_t   nextFreeIndex : 12; //
            std::uint32_t   version : 15;       // 29 bits
            std::uint32_t   active : 1;         //  total
            std::uint32_t   endOfList : 1;      //
            R*              resource;
            HandleEntry(){
                version = 1;
                nextFreeIndex = 1;
                active = endOfList = 0;
                resource = nullptr;
            }
            explicit HandleEntry(unsigned int _nextFreeIndex){
                nextFreeIndex = ++_nextFreeIndex;
                version = 1;
                active = endOfList = 0; 
                resource = nullptr;
            }
        };

        template<typename T> class ObjectPool final{
            private:
                unsigned int                  m_activeEntryCount;
                unsigned int                  m_firstFreeEntry;
                std::vector<HandleEntry<T>>   m_Pool;
            public:
                ObjectPool(const unsigned int numEntries){
                    m_Pool.resize(numEntries, HandleEntry<T>());
                    reset(); 
                }
                ~ObjectPool(){
                    for (size_t i = 0; i < m_Pool.size(); ++i) {
                        SAFE_DELETE(m_Pool[i].resource);
                    }
                }
                const unsigned int maxEntries(){
                    return static_cast<unsigned int>(m_Pool.size());
                }
                const unsigned int size(){
                    unsigned int c = 0;
                    unsigned int i;
                    for(i = 0; i < m_Pool.size(); ++i){
                        if(m_Pool[i].resource){ 
                            c = i; 
                        }
                    }
                    return c;
                } 
                void reset(){
                    m_activeEntryCount = 0;
                    m_firstFreeEntry = 1;
                    const auto lastIndex = m_Pool.size() - 1;
                    for (unsigned int i = 0; i < lastIndex; ++i){
                        m_Pool[i] = HandleEntry<T>(i + 1);
                    }
                    m_Pool[lastIndex] = HandleEntry<T>();
                    m_Pool[lastIndex].endOfList = true;
                }
                void update(Handle& _handle, T* _ptr){
                    const unsigned int index = _handle.index - 1;
                    if(m_Pool[index].version == _handle.version && m_Pool[index].active){
                        m_Pool[index].resource = _ptr;
                    }
                }
                void update(const unsigned int& _id, T* _ptr){
                    const unsigned int index = _id - 1;
                    if(m_Pool[index].active){
                        m_Pool[index].resource = _ptr;
                    }
                }
                template<typename R> Handle getHandle(R& resource) {
                    for (unsigned int i = 0; i < m_Pool.size(); ++i) {
                        R* res = dynamic_cast<R*>(m_Pool[i].resource);
                        if (res) {
                            if (res->name() == resource.name()) {
                                return Handle(i + 1, m_Pool[i].version, resource.type());
                            }
                        }
                    }
                    return Handle();
                }
                Handle add(T* _ptr, const unsigned int _type){
                    const unsigned int newIndex = m_firstFreeEntry - 1;
                    if(newIndex >= m_Pool.size())
                        return Handle(); //null handle
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
                const unsigned int add(T* _ptr){
                    const unsigned int newIndex = m_firstFreeEntry - 1;
                    if(newIndex >= m_Pool.size()) return 0; //null entity
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
                void remove(const unsigned int _index){
                    const unsigned int index = _index - 1;
                    if(m_Pool[index].active){
                        m_Pool[index].nextFreeIndex = m_firstFreeEntry;
                        m_Pool[index].active = false;
                        SAFE_DELETE(m_Pool[index].resource);
                        m_firstFreeEntry = _index;
                        --m_activeEntryCount;		
                    }
                }
                void remove(Handle& _handle){
                    const unsigned int index = _handle.index;
                    const unsigned int realIndex = index - 1;
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
                T* get(const unsigned int& _id){
                    T* outPtr = nullptr;
                    if (!get(_id, outPtr)) return nullptr;
                    return outPtr;
                }
                const bool get(const Handle& _handle, T*& _outPtr){
                    const unsigned int index = _handle.index - 1;
                    if (m_Pool[index].version != _handle.version || !m_Pool[index].active){
                        _outPtr = nullptr;
                        return false;
                    }
                    _outPtr = m_Pool[index].resource;
                    return true;
                }
                const bool get(const unsigned int& _index, T*& _outPtr){
                    const unsigned int index = _index - 1;
                    if(!m_Pool[index].resource){
                        _outPtr = nullptr;
                        return false;
                    }
                    _outPtr = m_Pool[index].resource;
                    return true;
                }
                template<typename U> inline const bool getAs(Handle& _handle, U*& _outPtr){
                    T* _void = nullptr;
                    const bool rv = get(_handle,_void);
                    _outPtr = (U*)_void; //use union_cast ? was in the original source
                    return rv;
                }
                template<typename U> inline const bool getAs(const unsigned int& _index, U*& _outPtr){
                    T* _void = nullptr;
                    const bool rv = get(_index,_void);
                    _outPtr = (U*)_void; //use union_cast ? was in the original source
                    return rv;
                }
                template<typename U> inline void getAsFast(const unsigned int& _index, U*& _outPtr){
                    const unsigned int index = _index - 1;
                    if(!m_Pool[index].resource){
                        _outPtr = nullptr;
                        return;
                    }
                    _outPtr = (U*)m_Pool[index].resource;
                }
                template<typename U> inline void getAsFast(Handle& _handle, U*& _outPtr){
                    const unsigned int index = _handle.index - 1;
                    if(!m_Pool[index].resource){
                        _outPtr = nullptr;
                        return;
                    }
                    _outPtr = (U*)m_Pool[index].resource;
                }
                template<typename U> inline U* getAsFast(Handle& _handle){
                    const unsigned int index = _handle.index - 1;
                    if(!m_Pool[index].resource) return nullptr;
                    return (U*)m_Pool[index].resource;
                }
                template<typename U> inline U* getAsFast(const unsigned int& _index){
                    const unsigned int index = _index - 1;
                    if(!m_Pool[index].resource) return nullptr;
                    return (U*)m_Pool[index].resource;
                }
        };
    };
};


#endif