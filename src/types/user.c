#include "types.h"
#include "error.h"

CN_API uint8_t user_copy(User *dest, const User *src)
{
    user_empty(dest);

    dest->is_bot = src->is_bot;
    dest->is_system = src->is_system;

    dest->avatar = src->avatar ? strdup(src->avatar) : NULL;
    dest->display_name = src->display_name ? strdup(src->display_name) : NULL;
    dest->id = src->id ? strdup(src->id) : NULL;
    dest->username = src->username ? strdup(src->username) : NULL;

    if (src->avatar && !dest->avatar) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new avatar.");
        return (1);
    }
    if (src->display_name && !dest->display_name) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new display_name.");
        return (1);
    }
    if (src->id && !dest->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new id.");
        return (1);
    }
    if (src->username && !dest->username) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new username.");
        return (1);
    }
    return (0);
}

CN_API User *user_dup(const User *src)
{
    User *dest = malloc(sizeof(User));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new user.");
        return (NULL);
    }

    memset(dest, 0, sizeof(User));

    if (user_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)user_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void user_empty(User *user)
{
    if (user->avatar) {
        (void)free(user->avatar);
        user->avatar = NULL;
    }
    if (user->id) {
        (void)free(user->id);
        user->id = NULL;
    }
    if (user->display_name) {
        (void)free(user->display_name);
        user->display_name = NULL;
    }
    if (user->username) {
        (void)free(user->username);
        user->username = NULL;
    }
}

CN_API void user_dedup(User *user)
{
    user_empty(user);
    (void)free(user);
}
