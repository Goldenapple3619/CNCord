#include "buffer.h"
#include "error.h"

CN_API Buffer *new_buffer(void)
{
    Buffer *vec = (Buffer *)malloc(sizeof(Buffer));

    if (!vec) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate buffer.")
        return (NULL);
    }

    vec->capacity = 0;
    vec->size = 0;
    vec->data = NULL;
    return (vec);
}

CN_API void init_buffer(Buffer *buf)
{
    if (!buf) {
        RAISE(ERR_INVALID_POINTER, "can't init an invalid buffer.");
        return;
    }

    buf->size = 0;
    buf->capacity = 0;
    buf->data = NULL;
}

CN_API void empty_buffer(Buffer *buf)
{
    if (!buf) {
        RAISE(ERR_INVALID_POINTER, "empty of an invalid buffer.");
        return;
    }

    if (buf->data) {
        (void)free(buf->data);
        buf->data = NULL;
    }

    buf->capacity = 0;
    buf->size = 0;
}

CN_API void delete_buffer(Buffer *buf)
{
    if (!buf) {
        RAISE(ERR_INVALID_POINTER, "delete of an invalid buffer.");
        return;
    }

    (void)empty_buffer(buf);
    (void)free(buf);
}

CN_API uint8_t resize_buffer(Buffer *buf, size_t new_capacity)
{
    if (!buf) {
        RAISE(ERR_INVALID_POINTER, "invalid resize on empty buffer.");
        return (1);
    }

    buf->data = realloc(buf->data, new_capacity * sizeof(char));

    if (!buf->data) {
        RAISE_FMT(ERR_OUT_OF_MEMORY, "failed to resize buffer (%zu -> %zu).", buf->capacity, new_capacity);
        buf->capacity = 0;
        buf->size = 0;
        return (1);
    }

    buf->capacity = new_capacity;

    return (0);
}

CN_API cnbool isempty_buffer(Buffer *buf)
{
    if (!buf) {
        RAISE(ERR_INVALID_POINTER, "invalid isempty on null buffer.");
        return (1);
    }

    if (!buf->data)
        return (true);

    if (!(*(buf->data)))
        return (true);
    return (false);
}

CN_API uint8_t add_buffer(Buffer *buf, const char *src, size_t size)
{
    if (!buf) {
        RAISE(ERR_INVALID_POINTER, "invalid add on empty buffer.");
        return (1);
    }

    if (!src) {
        RAISE(ERR_INVALID_POINTER, "str is empty.");
        return (1);
    }

    while (buf->size + size + 1 >= buf->capacity) {
        size_t new_capacity = buf->capacity == 0 ? 4096 : buf->capacity * 2;
        if (resize_buffer(buf, new_capacity)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    (void)memcpy(buf->data + buf->size, src, size);
    buf->size += size;
    buf->data[buf->size] = '\0';
    return (0);
}
