#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

static uint8_t role_put(Client *client, const cJSON *d, uint8_t (*cb)(Client *, const char *, const Role *))
{
    Role role = {0};
    Role *copy;
    Guild *guild;
    void *old = NULL;
    cnbool failed = false;

    if (!json_str(d, "guild_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (parse_role(&role, cJSON_GetObjectItemCaseSensitive(d, "role"))) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<ROLE id='%s' name='%s' color=%" PRIu64 " guild_id='%s'>", role.id, role.name, role.color, json_str(d, "guild_id"));

    copy = role_dup(&role);

    if (!copy) {
        PROPAGATE_ERR();
        return (1);
    }

    client_cache_lock(client);

    guild = find_guild_unlocked(client, json_str(d, "guild_id"));
    if (guild)
        failed = put_by_id(&guild->roles, copy, copy->id, &old);

    client_cache_unlock(client);

    if (old)
        role_dedup(old);
    if (!guild || failed)
        role_dedup(copy);

    if (failed) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to cache role.");
        return (1);
    }

    if (!guild)
        LOG_W("guild %s not found, skipping role caching.", json_str(d, "guild_id"));

    if (cb && cb(client, json_str(d, "guild_id"), &role)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_role_create(Client *client, const cJSON *d)
{
    return (role_put(client, d, client->events.on_role_create));
}

uint8_t on_role_update(Client *client, const cJSON *d)
{
    return (role_put(client, d, client->events.on_role_update));
}

uint8_t on_role_delete(Client *client, const cJSON *d)
{
    Guild *guild;
    Role *old = NULL;
    uint8_t status = 0;

    if (!json_str(d, "guild_id") || !json_str(d, "role_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    client_cache_lock(client);

    guild = find_guild_unlocked(client, json_str(d, "guild_id"));
    if (guild)
        old = take_by_id(&guild->roles, json_str(d, "role_id"));

    client_cache_unlock(client);

    if (!old) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "failed to delete role, %s not found in guild %s.", json_str(d, "role_id"), json_str(d, "guild_id"));
        return (1);
    }

    LOG_D("~<ROLE id='%s' name='%s' guild_id='%s'>", old->id, old->name ? old->name : "?", json_str(d, "guild_id"));

    if (client->events.on_role_delete)
        status = client->events.on_role_delete(client, json_str(d, "guild_id"), old);

    role_dedup(old);

    if (status) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}