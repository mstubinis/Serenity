#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <vector>
#include <boost/lexical_cast.hpp>

//removes a specific element from a vector
template<typename E,typename B> void removeFromVector(std::vector<B*>& v,E* e){
    for(auto it=v.begin();it!=v.end();){B* c=(*it);if(c==e){it=v.erase(it);}else{++it;}}
}

//clears a vector, reset its size to zero, and removes the elements from memory. does NOT delete pointer elements
template <typename E> void vector_clear(std::vector<E>& t){ t.clear(); std::vector<E>().swap(t); t.shrink_to_fit(); }
//converts any type to its string representation
template <typename E> std::string to_string(E t){ return boost::lexical_cast<std::string>(t); }
//formats a number to have commas to represent thousandth places
template<typename T> std::string convertNumToNumWithCommas(T n){std::string r=to_string(n);int p=r.length()-3;while(p>0){r.insert(p,",");p-=3;}return r;}

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { delete x; x = nullptr; } // Convenience macro for deleting a pointer
#define SAFE_DELETE_VECTOR(x){ for(auto it:x){ SAFE_DELETE(it); } } // Convenience macro for deleting the pointer elements of a vector
#define SAFE_DELETE_MAP(x){ for(auto it:x){ SAFE_DELETE(it.second); } } // Convenience macro for deleting the pointer elements of a map

#endif