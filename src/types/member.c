#include "types.h"
#include "error.h"

CN_API uint8_t member_copy(Member *dest, const Member *src)
{
    char *temp;

    member_empty(dest);

    if (user_copy(&dest->user, &src->user)) {
        PROPAGATE_ERR();
        return (1);
    }

    dest->id = src->id ? strdup(src->id) : NULL;
    dest->nick = src->nick ? strdup(src->nick) : NULL;
    dest->joined_at = src->joined_at ? strdup(src->joined_at) : NULL;

    if (src->id && !dest->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new id.");
        return (1);
    }
    if (src->nick && !dest->nick) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new nick.");
        return (1);
    }
    if (src->joined_at && !dest->joined_at) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new joined_at.");
        return (1);
    }

    for (size_t i = 0; i < src->roles.size; ++i) {
        temp = strdup(src->roles.content[i]);

        if (!temp) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new role id.");
            return (1);
        }

        if (insert_generic_vector(&dest->roles, temp)) {
            PROPAGATE_ERR();
            free(temp);
            return (1);
        }
    }

    return (0);
}

CN_API Member *member_dup(const Member *src)
{
    Member *dest = malloc(sizeof(Member));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new member.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Member));

    if (member_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)member_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void member_empty(Member *member)
{
    empty_generic_vector(&member->roles, &free);
    user_empty(&member->user);

    if (member->id) {
        (void)free(member->id);
        member->id = NULL;
    }
    if (member->nick) {
        (void)free(member->nick);
        member->nick = NULL;
    }
    if (member->joined_at) {
        (void)free(member->joined_at);
        member->joined_at = NULL;
    }
}

CN_API void member_dedup(Member *member)
{
    member_empty(member);
    (void)free(member);
}