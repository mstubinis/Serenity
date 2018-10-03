#pragma once
#ifndef ENGINE_ECS_REGISTRY_H
#define ENGINE_ECS_REGISTRY_H

#include <cstdint>
#include <boost/type_index.hpp>
#include <boost/unordered_map.hpp>

typedef std::uint32_t                    uint;
typedef boost::typeindex::type_index     boost_type_index;

template <typename T> const boost_type_index type_id() { return boost_type_index(boost::typeindex::type_id<T>()); }
template <typename T> const boost_type_index type_id(T* component) { return boost_type_index(boost::typeindex::type_id_runtime(*component)); }

class ECSRegistry{
	private:
        static uint lastIndex;
	    //maps type_info's to unsigned ints that will be used as array indices
	    static boost::unordered_map<boost_type_index,uint> slotMap;
		
	public:
        ECSRegistry(){
		}
		~ECSRegistry(){
			lastIndex = 0;
			slotMap.clear();
		}
		
		template<typename T> static void registerComponent(){	
			auto type = type_id<T>();
			if(slotMap.count(type)) return;	
			slotMap.emplace(type,lastIndex);		
			++lastIndex;
		}
};


#endif