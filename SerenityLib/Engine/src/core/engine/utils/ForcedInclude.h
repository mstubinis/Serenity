#pragma once
#ifndef ENGINE_FORCED_INCLUDE_H
#define ENGINE_FORCED_INCLUDE_H

#include <locale>
#include <stdint.h>

#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
    #define ALLOC new( _CLIENT_BLOCK, __FILE__, __LINE__)
    #define NEW ALLOC
    #define MALLOC(size) _malloc_dbg(size, _CLIENT_BLOCK, __FILE__, __LINE__)
    #define FREE(block) _free_dbg(block, _CLIENT_BLOCK)
    #define DEL delete
#else
    #define ALLOC new
    #define NEW ALLOC
    #define MALLOC malloc
    #define FREE free
    #define DEL delete
#endif

#endif

#define SAFE_DELETE_FUTURE(x){ \
    if (x) { \
        if (x->_Is_ready() || x->_Is_ready_at_thread_exit()) { \
            x->get(); \
        } \
        DEL x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_THREAD(x){ \
    if (x) { \
        if (x->joinable()) { \
            x->join(); \
        } \
        DEL x; \
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
        DEL x; \
        x = nullptr; \
    } \
}
#define SAFE_DELETE_VECTOR(x){ \
    if(x.size() > 0) { \
        for (size_t i = 0; i < x.size(); ++i) { \
            SAFE_DELETE(x[i]); \
        } \
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
    } \
}