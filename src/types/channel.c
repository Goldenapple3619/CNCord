#include "types.h"
#include "error.h"

CN_API uint8_t channel_copy(Channel *dest, const Channel *src)
{
    Message *temp;

    channel_empty(dest);

    dest->id = src->id ? strdup(src->id) : NULL;
    dest->name = src->name ? strdup(src->name) : NULL;
    dest->topic = src->topic ? strdup(src->topic) : NULL;
    dest->parent_id = src->parent_id ? strdup(src->parent_id) : NULL;
    dest->owner_id = src->owner_id ? strdup(src->owner_id) : NULL;
    dest->last_pin_timestamp = src->last_pin_timestamp ? strdup(src->last_pin_timestamp) : NULL;
    dest->type = src->type;
    dest->position = src->position;
    dest->nsfw = src->nsfw;
    dest->archived = src->archived;
    dest->locked = src->locked;

    if (src->id && !dest->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new id.");
        return (1);
    }
    if (src->name && !dest->name) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new name.");
        return (1);
    }
    if (src->topic && !dest->topic) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new topic.");
        return (1);
    }
    if (src->parent_id && !dest->parent_id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new parent_id.");
        return (1);
    }
    if (src->owner_id && !dest->owner_id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new owner_id.");
        return (1);
    }
    if (src->last_pin_timestamp && !dest->last_pin_timestamp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new last_pin_timestamp.");
        return (1);
    }

    for (size_t i = 0; i < src->messages.size; ++i) {
        temp = message_dup(src->messages.content[i]);

        if (!temp) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->messages, temp)) {
            PROPAGATE_ERR();
            message_dedup(temp);
            return (1);
        }
    }

    return (0);
}

CN_API Channel *channel_dup(const Channel *src)
{
    Channel *dest = malloc(sizeof(Channel));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new channel.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Channel));

    if (channel_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)channel_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void channel_empty(Channel *channel)
{
    empty_generic_vector(&channel->messages, (void (*)(void *))&message_dedup);

    if (channel->id) {
        (void)free(channel->id);
        channel->id = NULL;
    }
    if (channel->name) {
        (void)free(channel->name);
        channel->name = NULL;
    }
    if (channel->topic) {
        (void)free(channel->topic);
        channel->topic = NULL;
    }
    if (channel->parent_id) {
        (void)free(channel->parent_id);
        channel->parent_id = NULL;
    }
    if (channel->owner_id) {
        (void)free(channel->owner_id);
        channel->owner_id = NULL;
    }
    if (channel->last_pin_timestamp) {
        (void)free(channel->last_pin_timestamp);
        channel->last_pin_timestamp = NULL;
    }
    channel->type = 0;
    channel->position = 0;
    channel->nsfw = false;
    channel->archived = false;
    channel->locked = false;
}

CN_API void channel_dedup(Channel *channel)
{
    channel_empty(channel);
    (void)free(channel);
}