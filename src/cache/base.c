#include "client.h"
#include "cache.h"

Channel *find_channel_in(const struct generic_vector_s *channels, const char *id)
{
    for (size_t i = 0; i < channels->size; ++i) {
        Channel *channel = channels->content[i];

        if (!strcmp(channel->id, id))
            return (channel);
    }
    return (NULL);
}

size_t find_by_id(const struct generic_vector_s *vec, const char *id)
{
    if (!id)
        return (NOT_FOUND);

    for (size_t i = 0; i < vec->size; ++i) {
        const char *cached = *(char *const *)vec->content[i];

        if (cached && !strcmp(cached, id))
            return (i);
    }
    return (NOT_FOUND);
}

void *take_by_id(struct generic_vector_s *vec, const char *id)
{
    size_t i = find_by_id(vec, id);
    void *old;

    if (i == NOT_FOUND)
        return (NULL);

    old = vec->content[i];
    remove_generic_ordered_vector(vec, i, NULL);
    return (old);
}

uint8_t put_by_id(struct generic_vector_s *vec, void *fresh, const char *id, void **old)
{
    size_t i = find_by_id(vec, id);

    if (i == NOT_FOUND)
        return (insert_generic_vector(vec, fresh));

    *old = vec->content[i];
    vec->content[i] = fresh;
    return (0);
}

void swap_str(char **a, char **b)
{
    char *tmp = *a;

    *a = *b;
    *b = tmp;
}

void swap_vec(struct generic_vector_s *a, struct generic_vector_s *b)
{
    struct generic_vector_s tmp = *a;

    *a = *b;
    *b = tmp;
}