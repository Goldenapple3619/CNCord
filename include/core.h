#ifndef _CORE_H_
    #define _CORE_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <stdint.h>
    #include <string.h>
    #include <stdarg.h>
    #include <inttypes.h>

    /** ////////////////////// **/
    /**   true, false typing   **/
    /** ////////////////////// **/

    #define true ~(0 << 1)
    #define false 0

    /** ////////////////////// **/
    /**   windows dll export   **/
    /** ////////////////////// **/

    #ifdef _WIN32
        #define CN_API __declspec(dllexport)
    #else
        #define CN_API
    #endif

    /** ////////////////////////////// **/
    /**   primitive types definition   **/
    /** ////////////////////////////// **/

    typedef uint8_t cnbool;
#endif