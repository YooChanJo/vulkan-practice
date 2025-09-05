#pragma once

#ifdef INCLUDE_DEBUG_INFO
    #define DEBUG_ONLY(X) X
    #define RELEASE_ONLY(X)
#else
    #define DEBUG_ONLY(X)
    #define RELEASE_ONLY(X) X
#endif