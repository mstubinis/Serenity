#pragma once
#ifndef ENGINE_ECS_REGISTRY_H
#define ENGINE_ECS_REGISTRY_H

#include <cstdint>
#include <boost/type_index.hpp>
#include <boost/unordered_map.hpp>

typedef std::uint32_t                    uint;
typedef boost::typeindex::type_index     boost_type_index;

template <typename T> const boost_type_index type_ID() { return boost_type_index(boost::typeindex::type_id<T>()); }
template <typename T> const boost_type_index type_ID(T* t) { return boost_type_index(boost::typeindex::type_id_runtime(*t)); }

namespace Engine {
    namespace epriv {
        class ECSRegistry final{
            private:
                static uint lastIndex; static boost::unordered_map<boost_type_index, uint> slotMap;
            public:
                ECSRegistry() = default;
                ~ECSRegistry() { lastIndex = 0; slotMap.clear(); }
                template <typename T> static const uint type_slot() {
                    auto type = type_ID<T>(); if (!slotMap.count(type)) { slotMap.emplace(type, lastIndex); ++lastIndex; }
                    return slotMap.at(type);
                }
                template <typename T> static const uint type_slot(T* t) {
                    auto type = type_ID(t); if (!slotMap.count(type)) { slotMap.emplace(type, lastIndex); ++lastIndex; }
                    return slotMap.at(type);
                }
            };
    };
};

#endif