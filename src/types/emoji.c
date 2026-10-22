#include "types.h"
#include "error.h"

CN_API uint8_t emoji_copy(Emoji *dest, const Emoji *src)
{
    emoji_empty(dest);

    dest->animated = src->animated;

    dest->id = src->id ? strdup(src->id) : NULL;
    dest->name = src->name ? strdup(src->name) : NULL;

    if (src->id && !dest->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new id.");
        return (1);
    }
    if (src->name && !dest->name) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new name.");
        return (1);
    }

    return (0);
}

CN_API Emoji *emoji_dup(const Emoji *src)
{
    Emoji *dest = malloc(sizeof(Emoji));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new emoji.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Emoji));

    if (emoji_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)emoji_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void emoji_empty(Emoji *emoji)
{
    if (emoji->id) {
        (void)free(emoji->id);
        emoji->id = NULL;
    }
    if (emoji->name) {
        (void)free(emoji->name);
        emoji->name = NULL;
    }
    emoji->animated = false;
}

CN_API void emoji_dedup(Emoji *emoji)
{
    emoji_empty(emoji);
    (void)free(emoji);
}