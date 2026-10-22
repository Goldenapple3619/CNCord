#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

uint8_t on_guild_create(Client *client, const cJSON *d)
{
    Guild guild = {0};
    Guild *guild_duplicate;
    Guild *replaced = NULL;
    uint8_t status;

    const cJSON *roles = cJSON_GetObjectItemCaseSensitive(d, "roles");
    const cJSON *emojis = cJSON_GetObjectItemCaseSensitive(d, "emojis");
    const cJSON *channels = cJSON_GetObjectItemCaseSensitive(d, "channels");
    const cJSON *threads = cJSON_GetObjectItemCaseSensitive(d, "threads");
    const cJSON *members = cJSON_GetObjectItemCaseSensitive(d, "members");

    if (!json_str(d, "id") || !json_str(d, "name")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    guild.id = json_str(d, "id");
    guild.name = json_str(d, "name");
    guild.icon = json_str(d, "icon");
    guild.owner_id = json_str(d, "owner_id");
    guild.description = json_str(d, "description");
    guild.member_count = json_u64(d, "member_count");

    LOG_D("<GUILD id='%s', icon='%s', name='%s'>",
        guild.id, guild.icon ? guild.icon : "?", guild.name);

    guild_duplicate = guild_dup(&guild);

    if (!guild_duplicate) {
        PROPAGATE_ERR();
        return (1);
    }

    if ((cJSON_IsArray(roles) && load_roles(guild_duplicate, roles))
        || (cJSON_IsArray(emojis) && load_emojis(guild_duplicate, emojis))
        || (cJSON_IsArray(channels) && load_channels(guild_duplicate, channels))
        || (cJSON_IsArray(threads) && load_channels(guild_duplicate, threads))
        || (cJSON_IsArray(members) && load_members(guild_duplicate, members))) {
        PROPAGATE_ERR();
        guild_dedup(guild_duplicate);
        return (1);
    }

    client_cache_lock(client);
    status = put_guild_unlocked(client, guild_duplicate, &replaced);
    client_cache_unlock(client);

    if (replaced)
        guild_dedup(replaced);

    if (status) {
        PROPAGATE_ERR();
        guild_dedup(guild_duplicate);
        return (1);
    }

    FIRE(client, on_guild_create, guild_duplicate);
    return (0);
}

uint8_t on_guild_update(Client *client, const cJSON *d)
{
    Guild guild = {0};
    Guild *fresh;
    Guild *cached;

    const cJSON *roles = cJSON_GetObjectItemCaseSensitive(d, "roles");
    const cJSON *emojis = cJSON_GetObjectItemCaseSensitive(d, "emojis");

    if (!json_str(d, "id") || !json_str(d, "name") || !cJSON_IsArray(roles) || !cJSON_IsArray(emojis)) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    guild.id = json_str(d, "id");
    guild.name = json_str(d, "name");
    guild.icon = json_str(d, "icon");
    guild.owner_id = json_str(d, "owner_id");
    guild.description = json_str(d, "description");

    LOG_D("<GUILD UPDATE id='%s', icon='%s', name='%s'>", guild.id, guild.icon ? guild.icon : "?", guild.name);

    fresh = guild_dup(&guild);

    if (!fresh) {
        PROPAGATE_ERR();
        return (1);
    }

    if (load_roles(fresh, roles) || load_emojis(fresh, emojis)) {
        PROPAGATE_ERR();
        guild_dedup(fresh);
        return (1);
    }

    client_cache_lock(client);
    cached = find_guild_unlocked(client, fresh->id);
    if (cached)
        swap_guild_info_unlocked(cached, fresh);
    client_cache_unlock(client);

    if (!cached) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "failed to update guild, %s not found.", fresh->id);
        guild_dedup(fresh);
        return (1);
    }

    guild_dedup(fresh);

    FIRE(client, on_guild_update, cached);
    return (0);
}

uint8_t on_guild_delete(Client *client, const cJSON *d)
{
    Guild *old;
    uint8_t status = 0;

    if (!json_str(d, "id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (json_flag(d, "unavailable")) {
        LOG_W("guild %s is unavailable, keeping it cached.", json_str(d, "id"));
        return (0);
    }

    client_cache_lock(client);
    old = take_by_id(&client->cache.guilds, json_str(d, "id"));
    client_cache_unlock(client);

    if (!old) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "failed to delete guild, %s not found.", json_str(d, "id"));
        return (1);
    }

    LOG_D("~<GUILD id='%s' name='%s'>", old->id, old->name ? old->name : "?");

    if (client->events.on_guild_delete)
        status = client->events.on_guild_delete(client, old);

    guild_dedup(old);

    if (status) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_emojis_update(Client *client, const cJSON *d)
{
    Guild fresh = {0};
    Guild *guild;

    const cJSON *emojis = cJSON_GetObjectItemCaseSensitive(d, "emojis");

    if (!json_str(d, "guild_id") || !cJSON_IsArray(emojis)) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (load_emojis(&fresh, emojis)) {
        PROPAGATE_ERR();
        guild_empty(&fresh);
        return (1);
    }

    client_cache_lock(client);
    guild = find_guild_unlocked(client, json_str(d, "guild_id"));
    if (guild)
        swap_vec(&guild->emojis, &fresh.emojis);
    client_cache_unlock(client);

    guild_empty(&fresh);

    if (!guild) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "failed to update emojis, guild %s not found.", json_str(d, "guild_id"));
        return (1);
    }

    FIRE(client, on_emojis_update, guild);
    return (0);
}

uint8_t on_integrations_update(Client *client, const cJSON *d)
{
    if (!json_str(d, "guild_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    FIRE(client, on_integrations_update, json_str(d, "guild_id"));
    return (0);
}