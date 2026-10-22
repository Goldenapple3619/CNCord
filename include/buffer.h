#ifndef _BUFFER_H_
    #define _BUFFER_H_

    #include <stdlib.h>

    #include "core.h"

    typedef struct {
        char *data;
        size_t size;
        size_t capacity;
    } Buffer;

    CN_API Buffer *new_buffer(void);
    CN_API void init_buffer(Buffer *buf);
    CN_API void empty_buffer(Buffer *buf);
    CN_API void delete_buffer(Buffer *buf);
    CN_API uint8_t resize_buffer(Buffer *buf, size_t new_capacity);
    CN_API cnbool isempty_buffer(Buffer *buf);
    CN_API uint8_t add_buffer(Buffer *buf, const char *src, size_t size);
#endif
