#pragma once
#ifndef ENGINE_ECS_REGISTRY_H
#define ENGINE_ECS_REGISTRY_H

#include <boost/type_index.hpp>
#include <boost/unordered_map.hpp>

typedef boost::typeindex::type_index     boost_type_index;

template <typename T> const boost_type_index type_ID() { 
	return boost_type_index(boost::typeindex::type_id<T>()); 
}
template <typename T> const boost_type_index type_ID(T* t) { 
	return boost_type_index(boost::typeindex::type_id_runtime(*t)); 
}

namespace Engine::priv {
    class ECSRegistry final{
        private:
            static unsigned int                                          lastIndex;
            static boost::unordered_map<boost_type_index, unsigned int>  slotMap;
        public:
            ECSRegistry() {}
            ECSRegistry(const ECSRegistry&)                      = delete;
            ECSRegistry& operator=(const ECSRegistry&)           = delete;
            ECSRegistry(ECSRegistry&& other) noexcept            = delete;
            ECSRegistry& operator=(ECSRegistry&& other) noexcept = delete;
            ~ECSRegistry() { 
                lastIndex = 0;
                slotMap.clear(); 
            }
            template <typename T> static const unsigned int& type_slot() {
                const auto& type = type_ID<T>();
                if (!slotMap.count(type)) { 
                    slotMap.emplace(type, lastIndex); 
                    ++lastIndex; 
                }
                return slotMap.at(type);
            }
            template <typename T> static const unsigned int& type_slot(T* t) {
                const auto& type = type_ID(t); 
                if (!slotMap.count(type)) { 
                    slotMap.emplace(type, lastIndex); 
                    ++lastIndex; 
                }
                return slotMap.at(type);
            }
			template <typename T> static const unsigned int& type_slot_fast() {
				const auto& type = type_ID<T>();
				return slotMap.at(type);
			}
			template <typename T> static const unsigned int& type_slot_fast(T* t) {
				const auto& type = type_ID(t);
				return slotMap.at(type);
			}
        };
};

#endif