#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

static uint8_t member_put(Client *client, const cJSON *d, cnbool is_new,
    uint8_t (*cb)(Client *, const char *, const Member *))
{
    Member memb = {0};
    Member *copy;
    Guild *guild;
    void *old = NULL;
    uint8_t status = 0;
    cnbool failed = false;

    if (!json_str(d, "guild_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (parse_member(&memb, d)) {
        PROPAGATE_ERR();
        return (1);
    }

    copy = member_dup(&memb);
    empty_generic_vector(&memb.roles, &free);

    if (!copy) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<MEMBER id='%s' username='%s' guild_id='%s'>", copy->id, copy->user.username ? copy->user.username : "?", json_str(d, "guild_id"));

    client_cache_lock(client);

    guild = find_guild_unlocked(client, json_str(d, "guild_id"));
    if (guild) {
        failed = put_by_id(&guild->members, copy, copy->id, &old);

        if (!failed && is_new)
            guild->member_count++;
    }

    client_cache_unlock(client);

    if (old)
        member_dedup(old);

    if (failed) {
        member_dedup(copy);
        RAISE(ERR_OUT_OF_MEMORY, "failed to cache member.");
        return (1);
    }

    if (!guild)
        LOG_W("guild %s not found, skipping member caching.", json_str(d, "guild_id"));

    if (cb)
        status = cb(client, json_str(d, "guild_id"), copy);

    if (!guild)
        member_dedup(copy);

    if (status) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_member_add(Client *client, const cJSON *d)
{
    return (member_put(client, d, true, client->events.on_member_add));
}

uint8_t on_member_update(Client *client, const cJSON *d)
{
    return (member_put(client, d, false, client->events.on_member_update));
}

uint8_t on_member_remove(Client *client, const cJSON *d)
{
    Member fallback = {0};
    Guild *guild;
    Member *old = NULL;
    uint8_t status = 0;

    if (!json_str(d, "guild_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (parse_user(&fallback.user, cJSON_GetObjectItemCaseSensitive(d, "user"), NULL)) {
        PROPAGATE_ERR();
        return (1);
    }

    fallback.id = fallback.user.id;

    LOG_D("~<MEMBER id='%s' username='%s' guild_id='%s'>", fallback.id, fallback.user.username ? fallback.user.username : "?", json_str(d, "guild_id"));

    client_cache_lock(client);

    guild = find_guild_unlocked(client, json_str(d, "guild_id"));
    if (guild) {
        old = take_by_id(&guild->members, fallback.id);

        if (guild->member_count)
            guild->member_count--;
    }

    client_cache_unlock(client);

    if (client->events.on_member_remove)
        status = client->events.on_member_remove(client, json_str(d, "guild_id"), old ? old : &fallback);

    if (old)
        member_dedup(old);

    if (status) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

static uint8_t ban_event(Client *client, const cJSON *d, uint8_t (*cb)(Client *, const char *, const User *))
{
    User user = {0};

    if (!json_str(d, "guild_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (parse_user(&user, cJSON_GetObjectItemCaseSensitive(d, "user"), NULL)) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<BAN user_id='%s' guild_id='%s'>", user.id, json_str(d, "guild_id"));

    if (cb && cb(client, json_str(d, "guild_id"), &user)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_ban_add(Client *client, const cJSON *d)
{
    return (ban_event(client, d, client->events.on_ban_add));
}

uint8_t on_ban_remove(Client *client, const cJSON *d)
{
    return (ban_event(client, d, client->events.on_ban_remove));
}