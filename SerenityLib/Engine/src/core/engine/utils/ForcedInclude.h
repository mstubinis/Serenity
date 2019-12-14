#pragma once
#ifndef ENGINE_FORCED_INCLUDE_H
#define ENGINE_FORCED_INCLUDE_H

#include <locale>
#include <stdint.h>

#ifdef _DEBUG

// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the allocations to be of _CLIENT_BLOCK type

    #define _CRTDBG_MAP_ALLOC
    #define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__)
    #define MALLOC(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define FREE(block) _free_dbg(block, _NORMAL_BLOCK)
  
/*
    #define NEW new
    #define MALLOC malloc
    #define FREE free
    */

#else
    #define NEW new
    #define MALLOC malloc
    #define FREE free
#endif

#endif

#define SAFE_DELETE_FUTURE(x){ \
    if (x) { \
        if (x->_Is_ready() || x->_Is_ready_at_thread_exit()) { \
            x->get(); \
        } \
        delete x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_THREAD(x){ \
    if (x) { \
        if (x->joinable()) { \
            x->join(); \
        } \
        delete x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_COM(x) { \
    if(x){ \
        x->Release(); \
        x = 0; \
    } \
}
#define SAFE_DELETE(x) { \
    if(x){ \
        delete x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_VECTOR(x){ \
    if(x.size() > 0) { \
        for (size_t i = 0; i < x.size(); ++i) { \
            SAFE_DELETE(x[i]); \
        } \
        x.clear(); \
    } \
}
#define SAFE_DELETE_QUEUE(x){ \
    while (x.size() > 0) { \
        auto& val = x.front(); \
        SAFE_DELETE(val); \
        x.pop(); \
    } \
}
#define SAFE_DELETE_MAP(x){ \
    if(x.size() > 0){ \
        for(auto& it : x){ \
            SAFE_DELETE(it.second); \
        } \
        x.clear(); \
    } \
}