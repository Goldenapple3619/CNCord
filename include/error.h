#ifndef _ERROR_H_
    #define _ERROR_H_

    #include "core.h"

    /** ////////////////////// **/
    /**   some configuration   **/
    /** ////////////////////// **/

    #define ERR_MAX_FRAMES 32
    #define ERR_MSG_SIZE 128
    #define ERR_HISTORY 128

    #define ERR_FULL_TRACE 1

    /** ////////////////////////// **/
    /**   error reporting macros   **/
    /** ////////////////////////// **/

    #if defined(ERR_FULL_TRACE) && ERR_FULL_TRACE == 1
        #define RAISE(c, msg) raise_error(c, msg, __FILE__, __func__, __LINE__);
        #define RAISE_FMT(c, ...) raise_error_fmt(c, __FILE__, __func__, __LINE__, __VA_ARGS__);
        #define PROPAGATE_ERR() push_error(__FILE__, __func__, __LINE__);
    #elif defined(ERR_SEMI_TRACE) && ERR_SEMI_TRACE == 1
        #define RAISE(c, msg) raise_error(c, msg, "???", __func__, __LINE__);
        #define RAISE_FMT(c, ...) raise_error_fmt(c, "???", __func__, __LINE__, __VA_ARGS__);
        #define PROPAGATE_ERR() push_error("???", __func__, __LINE__);
    #else
        #define RAISE(c, msg) raise_error(c, msg, "???", "???", "???");
        #define RAISE_FMT(c, ...) raise_error_fmt(c, "???", "???", "???", __VA_ARGS__);
        #define PROPAGATE_ERR() push_error("???", "???", "???");
    #endif

    typedef enum {
        ERR_OK = 0,

        ERR_OUT_OF_MEMORY,
        ERR_INVALID_POINTER,
        ERR_OS,
        ERR_OUT_OF_BOUND,
        ERR_INVALID_TYPE,
        ERR_NOT_COMPATIBLE,
        ERR_RUNTIME,
        ERR_CORRUPT_OR_INVALID,

        WAR_IMPORTANT = 0xffff
    } ErrorCode;

    typedef struct {
        const char *file;
        const char *function;
        uint32_t line;
    } ErrorFrame;

    typedef struct {
        ErrorCode code;

        char message[ERR_MSG_SIZE];

        uint32_t frame_count;

        ErrorFrame frames[ERR_MAX_FRAMES];
    } ErrorContext;

    CN_API void raise_error(ErrorCode c, const char *message, const char *file, const char *function, uint32_t line);
    CN_API void push_error(const char *file, const char *function, uint32_t line);
    CN_API const ErrorContext *get_error(void);
    CN_API cnbool has_error(void);
    CN_API void print_error(const ErrorContext *err, FILE *output);
    CN_API const char *error_type_to_text(ErrorCode c);

    #if defined(__MINGW32__) || defined(__MINGW64__)
        CN_API void raise_error_fmt(ErrorCode c, const char *file, const char *function, uint32_t line, const char *fmt, ...) __attribute__((format(gnu_printf, 5, 6)));
    #elif defined(__GNUC__) || defined(__clang__)
        CN_API void raise_error_fmt(ErrorCode c, const char *file, const char *function, uint32_t line, const char *fmt, ...) __attribute__((format(printf, 5, 6)));
    #else
        CN_API void raise_error_fmt(ErrorCode c, const char *file, const char *function, uint32_t line, const char *fmt, ...);
    #endif

#endif
