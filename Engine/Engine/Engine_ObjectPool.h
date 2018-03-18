#pragma once
#ifndef ENGINE_OBJECT_POOL_H
#define ENGINE_OBJECT_POOL_H

#include <memory>
#include <cstdint>

#include "Engine_ResourceHandle.h"

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
		template<typename T>
		class ObjectPool final{
		    private:
				uint MAX_ENTRIES;
				HandleEntry<T>* m_Pool;
				uint m_activeEntryCount;
				uint32 m_firstFreeEntry;
		    public:
				ObjectPool(uint numEntries){ 
					MAX_ENTRIES = numEntries; m_Pool = new HandleEntry<T>[MAX_ENTRIES]; reset(); 
				}
				~ObjectPool(){ destruct(); }

				void destruct(){
					for(uint i = 0; i < MAX_ENTRIES; ++i){
						if(m_Pool[i].resource){
						    delete(m_Pool[i].resource);
						    m_Pool[i].resource = nullptr;
						}
					}
					delete[] m_Pool;
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
					if(m_Pool[index].counter == h.counter && m_Pool[index].active == true){
						m_Pool[index].resource = ptr;
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
				void remove(uint& index){
					if(m_Pool[index].active == true){
						m_Pool[index].nextFreeIndex = m_firstFreeEntry;
						m_Pool[index].active = false;
						if(m_Pool[index].resource != nullptr){
						    delete(m_Pool[index].resource);
						    m_Pool[index].resource = nullptr;
						}
						m_firstFreeEntry = index;
						--m_activeEntryCount;		
					}
				}
				void remove(Handle& handle){
					const uint32 index = handle.index;
					if(m_Pool[index].counter == handle.counter && m_Pool[index].active == true){
						m_Pool[index].nextFreeIndex = m_firstFreeEntry;
						m_Pool[index].active = false;
						delete(m_Pool[index].resource);
						m_Pool[index].resource = nullptr;
						m_firstFreeEntry = index;
						--m_activeEntryCount;		
					}
				}
				T* get(Handle& handle){
					T* outPtr = nullptr;
					if (!get(handle, outPtr)) return nullptr;
					return outPtr;
				}

				bool get(const Handle& handle, T*& outPtr){
					const uint index = handle.index;
					if (m_Pool[index].counter != handle.counter || m_Pool[index].active == false)
						return false;
					outPtr = m_Pool[index].resource;
					return true;
				}
				bool get(const uint& index, T*& outPtr){
					outPtr = m_Pool[index].resource;
					return true;
				}
				template<typename U> inline bool getAs(Handle& handle, U*& outPtr){
					T* _void;
					const bool rv = get(handle,_void);
					outPtr = (U*)_void; //use union_cast ? was in the original source
					return rv;
				}
				template<typename U> inline bool getAs(uint& index, U*& outPtr){
					T* _void;
					const bool rv = get(index,_void);
					outPtr = (U*)_void; //use union_cast ? was in the original source
					return rv;
				}
				template<typename U> inline void getAsFast(Handle& handle, U*& outPtr){
					outPtr = (U*)m_Pool[handle.index].resource;
				}
				template<typename U> inline void getAsFast(uint& index, U*& outPtr){
					outPtr = (U*)m_Pool[index].resource;
				}
				template<typename U> inline U* getAsFast(Handle& handle){
					return (U*)m_Pool[handle.index].resource;
				}
				template<typename U> inline U* getAsFast(uint& index){
					return (U*)m_Pool[index].resource;
				}
		};
	};
};


#endif