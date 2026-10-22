#ifndef _GENERIC_VECTOR_H_
    #define _GENERIC_VECTOR_H_

    #include <stdlib.h>

    #include "core.h"

    struct generic_vector_s {
        void **content;

        size_t size;
        size_t capacity;
    };

    CN_API void init_generic_vector(struct generic_vector_s *vec);
    CN_API struct generic_vector_s *new_generic_vector(void);
    CN_API uint8_t resize_generic_vector(struct generic_vector_s *vec, size_t new_capacity);
    CN_API uint8_t insert_generic_vector(struct generic_vector_s *vec, void *obj);
    CN_API void remove_generic_vector(struct generic_vector_s *vec, size_t i, void (*_delete_obj)(void *));
    CN_API void remove_generic_ordered_vector(struct generic_vector_s *vec, size_t i, void (*_delete_obj)(void *));
    CN_API void delete_generic_vector(struct generic_vector_s *vec, void (*_delete_obj)(void *));
    CN_API void empty_generic_vector(struct generic_vector_s *vec, void (*_delete_obj)(void *));
#endif