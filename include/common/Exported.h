#pragma once

#ifdef _WIN32
    #ifdef BUILDING_SHARED_LIB
        #define EXPORTED __declspec(dllexport)
    #else
        #define EXPORTED __declspec(dllimport)
    #endif
#else
    #define EXPORTED __attribute__((visibility("default")))
#endif
