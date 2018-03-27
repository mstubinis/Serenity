#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <vector>
#include <boost/lexical_cast.hpp>

template<typename E,typename B> void removeFromVector(std::vector<B*>& vector,E* element){
	for(auto it = vector.begin(); it != vector.end();){ B* c = (*it); if(c == element){ it = vector.erase(it); } else{ ++it; } }
}


template <typename E> void vector_clear(std::vector<E>& t){ t.clear(); std::vector<E>().swap(t); t.shrink_to_fit(); }
template <typename E> std::string to_string(E t){ return boost::lexical_cast<std::string>(t); }

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { delete x; x = nullptr; } // Convenience macro for deleting a pointer


#endif