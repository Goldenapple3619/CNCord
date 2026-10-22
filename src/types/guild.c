#include "types.h"
#include "error.h"

CN_API uint8_t guild_copy(Guild *dest, const Guild *src)
{
    guild_empty(dest);

    Role *temp_role;
    Emoji *temp_emoji;
    Channel *temp;
    Member *temp_member;

    dest->id = src->id ? strdup(src->id) : NULL;
    dest->name = src->name ? strdup(src->name) : NULL;
    dest->icon = src->icon ? strdup(src->icon) : NULL;
    dest->owner_id = src->owner_id ? strdup(src->owner_id) : NULL;
    dest->description = src->description ? strdup(src->description) : NULL;
    dest->member_count = src->member_count;

    if (src->id && !dest->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new id.");
        return (1);
    }
    if (src->name && !dest->name) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new name.");
        return (1);
    }
    if (src->icon && !dest->icon) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new icon.");
        return (1);
    }
    if (src->owner_id && !dest->owner_id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new owner_id.");
        return (1);
    }
    if (src->description && !dest->description) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new description.");
        return (1);
    }

    for (size_t i = 0; i < src->roles.size; ++i) {
        temp_role = role_dup(src->roles.content[i]);

        if (!temp_role) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->roles, temp_role)) {
            PROPAGATE_ERR();
            role_dedup(temp_role);
            return (1);
        }
    }

    for (size_t i = 0; i < src->emojis.size; ++i) {
        temp_emoji = emoji_dup(src->emojis.content[i]);

        if (!temp_emoji) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->emojis, temp_emoji)) {
            PROPAGATE_ERR();
            emoji_dedup(temp_emoji);
            return (1);
        }
    }

    for (size_t i = 0; i < src->channels.size; ++i) {
        temp = channel_dup(src->channels.content[i]);

        if (!temp) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->channels, temp)) {
            PROPAGATE_ERR();
            channel_dedup(temp);
            return (1);
        }
    }

    for (size_t i = 0; i < src->members.size; ++i) {
        temp_member = member_dup(src->members.content[i]);

        if (!temp_member) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->members, temp_member)) {
            PROPAGATE_ERR();
            member_dedup(temp_member);
            return (1);
        }
    }

    return (0);
}

CN_API Guild *guild_dup(const Guild *src)
{
    Guild *dest = malloc(sizeof(Guild));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new guild.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Guild));

    if (guild_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)guild_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void guild_empty(Guild *guild)
{
    empty_generic_vector(&guild->roles, (void (*)(void *))&role_dedup);
    empty_generic_vector(&guild->emojis, (void (*)(void *))&emoji_dedup);
    empty_generic_vector(&guild->channels, (void (*)(void *))&channel_dedup);
    empty_generic_vector(&guild->members, (void (*)(void *))&member_dedup);

    if (guild->id) {
        (void)free(guild->id);
        guild->id = NULL;
    }
    if (guild->icon) {
        (void)free(guild->icon);
        guild->icon = NULL;
    }
    if (guild->name) {
        (void)free(guild->name);
        guild->name = NULL;
    }
    if (guild->owner_id) {
        (void)free(guild->owner_id);
        guild->owner_id = NULL;
    }
    if (guild->description) {
        (void)free(guild->description);
        guild->description = NULL;
    }
    guild->member_count = 0;
}

CN_API void guild_dedup(Guild *guild)
{
    guild_empty(guild);
    (void)free(guild);
}