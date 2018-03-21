#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <vector>

template<typename E,typename B> void removeFromVector(std::vector<B*>& vector,E* element){
	for(auto it = vector.begin(); it != vector.end();){
		B* c = (*it);
		if(c == element){
			it = vector.erase(it);
		}
		else{
			++it;
		}
	}
}


#endif