#include "types.h"
#include "error.h"

CN_API uint8_t role_copy(Role *dest, const Role *src)
{
    role_empty(dest);

    dest->color = src->color;
    dest->permissions = src->permissions;
    dest->position = src->position;
    dest->hoist = src->hoist;
    dest->managed = src->managed;
    dest->mentionable = src->mentionable;

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

CN_API Role *role_dup(const Role *src)
{
    Role *dest = malloc(sizeof(Role));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new role.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Role));

    if (role_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)role_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void role_empty(Role *role)
{
    if (role->id) {
        (void)free(role->id);
        role->id = NULL;
    }
    if (role->name) {
        (void)free(role->name);
        role->name = NULL;
    }
    role->color = 0;
    role->permissions = 0;
    role->position = 0;
    role->hoist = false;
    role->managed = false;
    role->mentionable = false;
}

CN_API void role_dedup(Role *role)
{
    role_empty(role);
    (void)free(role);
}