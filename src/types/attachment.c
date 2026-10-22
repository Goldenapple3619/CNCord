#include "types.h"
#include "error.h"

CN_API uint8_t attachment_copy(Attachment *dest, const Attachment *src)
{
    attachment_empty(dest);

    dest->size = src->size;

    dest->id = src->id ? strdup(src->id) : NULL;
    dest->filename = src->filename ? strdup(src->filename) : NULL;
    dest->url = src->url ? strdup(src->url) : NULL;
    dest->content_type = src->content_type ? strdup(src->content_type) : NULL;

    if (src->id && !dest->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new id.");
        return (1);
    }
    if (src->filename && !dest->filename) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new filename.");
        return (1);
    }
    if (src->url && !dest->url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new url.");
        return (1);
    }
    if (src->content_type && !dest->content_type) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new content_type.");
        return (1);
    }

    return (0);
}

CN_API Attachment *attachment_dup(const Attachment *src)
{
    Attachment *dest = malloc(sizeof(Attachment));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new attachment.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Attachment));

    if (attachment_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)attachment_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void attachment_empty(Attachment *attachment)
{
    if (attachment->id) {
        (void)free(attachment->id);
        attachment->id = NULL;
    }
    if (attachment->filename) {
        (void)free(attachment->filename);
        attachment->filename = NULL;
    }
    if (attachment->url) {
        (void)free(attachment->url);
        attachment->url = NULL;
    }
    if (attachment->content_type) {
        (void)free(attachment->content_type);
        attachment->content_type = NULL;
    }
    attachment->size = 0;
}

CN_API void attachment_dedup(Attachment *attachment)
{
    attachment_empty(attachment);
    (void)free(attachment);
}