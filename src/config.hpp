#pragma once

#include <raylib.h>

#include <cstdint>
#include <cstdlib>
#include <cmath>

template<class... Ts> struct visitor : Ts... { using Ts::operator()...;  };
template<class... Ts> visitor(Ts...) -> visitor<Ts...>;


#if defined(_MSC_VER)
    #define DISABLE_WARNING_PUSH                __pragma(warning( push ))
    #define DISABLE_WARNING_POP                 __pragma(warning( pop ))
    #define DISABLE_WARNING(warningNumber)      __pragma(warning( disable : warningNumber ))
    #define DISABLE_WARNING_UNUSED_PARAMETER    DISABLE_WARNING(4100)
    #define DISABLE_WARNING_UNUSED_FUNCTION     DISABLE_WARNING(4505)
    #define DISABLE_WARNING_PEDANTIC
    #define DISABLE_WARNING_CONVERSION
    #define DISABLE_WARNING_ENUM_COMPARE
    #define DISABLE_WARNING_USELESS_CAST
    #define DISABLE_WARNING_DUPLICATE_BRANCHES
    #define DISABLE_WARNING_NARROWING

#elif defined(__clang__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH                DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP                 DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName)        DO_PRAGMA(GCC diagnostic ignored #warningName)
    #define DISABLE_WARNING_UNUSED_PARAMETER    DISABLE_WARNING(-Wunused-parameter)
    #define DISABLE_WARNING_UNUSED_FUNCTION     DISABLE_WARNING(-Wunused-function)
    #define DISABLE_WARNING_PEDANTIC            DISABLE_WARNING(-Wpedantic)
    #define DISABLE_WARNING_CONVERSION          DISABLE_WARNING(-Wconversion)
    #define DISABLE_WARNING_ENUM_COMPARE        DISABLE_WARNING(-Wenum-compare)
    #define DISABLE_WARNING_USELESS_CAST
    #define DISABLE_WARNING_DUPLICATE_BRANCHES
    #define DISABLE_WARNING_NARROWING           DISABLE_WARNING(-Wnarrowing)
#elif defined(__GNUC__)
    #define DO_PRAGMA(X) _Pragma(#X)
    #define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
    #define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
    #define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

    #define DISABLE_WARNING_UNUSED_PARAMETER    DISABLE_WARNING(-Wunused-parameter)
    #define DISABLE_WARNING_UNUSED_FUNCTION     DISABLE_WARNING(-Wunused-function)
    #define DISABLE_WARNING_PEDANTIC            DISABLE_WARNING(-Wpedantic)
    #define DISABLE_WARNING_CONVERSION          DISABLE_WARNING(-Wconversion)
    #define DISABLE_WARNING_ENUM_COMPARE        DISABLE_WARNING(-Wenum-compare)
    #define DISABLE_WARNING_USELESS_CAST        DISABLE_WARNING(-Wuseless-cast)
    #define DISABLE_WARNING_DUPLICATE_BRANCHES  DISABLE_WARNING(-Wduplicated-branches)
    #define DISABLE_WARNING_NARROWING           DISABLE_WARNING(-Wnarrowing)
#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING_UNUSED_PARAMETER
    #define DISABLE_WARNING_UNUSED_FUNCTION
    #define DISABLE_WARNING_PEDANTIC
    #define DISABLE_WARNING_CONVERSION
    #define DISABLE_WARNING_ENUM_COMPARE
    #define DISABLE_WARNING_USELESS_CAST
    #define DISABLE_WARNING_DUPLICATE_BRANCHES
    #define DISABLE_WARNING_NARROWING
#endif
