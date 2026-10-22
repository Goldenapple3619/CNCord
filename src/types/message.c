#include "types.h"
#include "error.h"

CN_API uint8_t message_copy(Message *dest, const Message *src)
{
    User *temp_user;
    Attachment *temp_attachment;
    Embed *temp_embed;
    char *temp_role;

    message_empty(dest);

    dest->type = src->type;
    dest->pinned = src->pinned;
    dest->mention_everyone = src->mention_everyone;

    if (user_copy(&dest->author, &src->author)) {
        PROPAGATE_ERR();
        return (1);
    }

    dest->id = src->id ? strdup(src->id) : NULL;
    dest->channel_id = src->channel_id ? strdup(src->channel_id) : NULL;
    dest->guild_id = src->guild_id ? strdup(src->guild_id) : NULL;
    dest->content = src->content ? strdup(src->content) : NULL;
    dest->timestamp = src->timestamp ? strdup(src->timestamp) : NULL;
    dest->edited_timestamp = src->edited_timestamp ? strdup(src->edited_timestamp) : NULL;
    dest->reply_to_id = src->reply_to_id ? strdup(src->reply_to_id) : NULL;

    if (src->id && !dest->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new id.");
        return (1);
    }
    if (src->channel_id && !dest->channel_id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new channel_id.");
        return (1);
    }
    if (src->guild_id && !dest->guild_id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new guild_id.");
        return (1);
    }
    if (src->content && !dest->content) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new content.");
        return (1);
    }
    if (src->timestamp && !dest->timestamp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new timestamp.");
        return (1);
    }
    if (src->edited_timestamp && !dest->edited_timestamp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new edited_timestamp.");
        return (1);
    }
    if (src->reply_to_id && !dest->reply_to_id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new reply_to_id.");
        return (1);
    }

    for (size_t i = 0; i < src->mentions.size; ++i) {
        temp_user = user_dup(src->mentions.content[i]);

        if (!temp_user) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->mentions, temp_user)) {
            PROPAGATE_ERR();
            user_dedup(temp_user);
            return (1);
        }
    }

    for (size_t i = 0; i < src->mention_roles.size; ++i) {
        temp_role = strdup(src->mention_roles.content[i]);

        if (!temp_role) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new role id.");
            return (1);
        }

        if (insert_generic_vector(&dest->mention_roles, temp_role)) {
            PROPAGATE_ERR();
            free(temp_role);
            return (1);
        }
    }

    for (size_t i = 0; i < src->attachments.size; ++i) {
        temp_attachment = attachment_dup(src->attachments.content[i]);

        if (!temp_attachment) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->attachments, temp_attachment)) {
            PROPAGATE_ERR();
            attachment_dedup(temp_attachment);
            return (1);
        }
    }

    for (size_t i = 0; i < src->embeds.size; ++i) {
        temp_embed = embed_dup(src->embeds.content[i]);

        if (!temp_embed) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->embeds, temp_embed)) {
            PROPAGATE_ERR();
            embed_dedup(temp_embed);
            return (1);
        }
    }

    return (0);
}

CN_API Message *message_dup(const Message *src)
{
    Message *dest = malloc(sizeof(Message));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new message.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Message));

    if (message_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)message_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void message_empty(Message *message)
{
    empty_generic_vector(&message->mentions, (void (*)(void *))&user_dedup);
    empty_generic_vector(&message->mention_roles, &free);
    empty_generic_vector(&message->attachments, (void (*)(void *))&attachment_dedup);
    empty_generic_vector(&message->embeds, (void (*)(void *))&embed_dedup);

    user_empty(&message->author);

    if (message->channel_id) {
        (void)free(message->channel_id);
        message->channel_id = NULL;
    }
    if (message->id) {
        (void)free(message->id);
        message->id = NULL;
    }
    if (message->guild_id) {
        (void)free(message->guild_id);
        message->guild_id = NULL;
    }
    if (message->content) {
        (void)free(message->content);
        message->content = NULL;
    }
    if (message->timestamp) {
        (void)free(message->timestamp);
        message->timestamp = NULL;
    }
    if (message->edited_timestamp) {
        (void)free(message->edited_timestamp);
        message->edited_timestamp = NULL;
    }
    if (message->reply_to_id) {
        (void)free(message->reply_to_id);
        message->reply_to_id = NULL;
    }
    message->type = 0;
    message->pinned = false;
    message->mention_everyone = false;
}

CN_API void message_dedup(Message *message)
{
    message_empty(message);
    (void)free(message);
}
