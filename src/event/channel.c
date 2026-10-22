#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

static uint8_t channel_put(Client *client, const cJSON *d, const char *default_guild_id,
    uint8_t (*cb)(Client *, const Channel *))
{
    Channel channel = {0};
    Channel *channel_duplicate;
    Channel *old = NULL;
    uint8_t status;

    const char *guild_id = json_str(d, "guild_id") ? json_str(d, "guild_id") : default_guild_id;

    if (parse_channel(&channel, d)) {
        PROPAGATE_ERR();
        return (1);
    }

    channel_duplicate = channel_dup(&channel);

    if (!channel_duplicate) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<CHANNEL id='%s' name='%s' type=%d guild_id='%s'>", channel.id, channel.name ? channel.name : "?", channel.type, guild_id ? guild_id : "null");

    client_cache_lock(client);
    status = put_channel_unlocked(client, guild_id, channel_duplicate, &old);
    client_cache_unlock(client);

    if (old)
        channel_dedup(old);

    if (status) {
        PROPAGATE_ERR();
        channel_dedup(channel_duplicate);
        return (1);
    }

    if (cb && cb(client, channel_duplicate)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

static uint8_t channel_remove(Client *client, const cJSON *d, uint8_t (*cb)(Client *, const Channel *))
{
    Channel *channel_old = NULL;
    uint8_t status;

    if (!json_str(d, "id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    client_cache_lock(client);
    status = remove_channel_unlocked(client, json_str(d, "guild_id"), json_str(d, "id"), &channel_old);
    client_cache_unlock(client);

    if (status) {
        PROPAGATE_ERR();
        if (channel_old)
            channel_dedup(channel_old);
        return (1);
    }

    if (!channel_old) {
        RAISE(ERR_RUNTIME, "failed to fetch channel");
        return (1);
    }

    LOG_D("~<CHANNEL id='%s' name='%s' type=%d guild_id='%s'>", channel_old->id, channel_old->name ? channel_old->name : "?", channel_old->type, json_str(d, "guild_id") ? json_str(d, "guild_id") : "null");

    if (cb) {
        if (cb(client, channel_old)) {
            channel_dedup(channel_old);
            PROPAGATE_ERR();
            return (1);
        }
    }
    channel_dedup(channel_old);
    return (0);
}

uint8_t on_channel_create(Client *client, const cJSON *d)
{
    return (channel_put(client, d, NULL, client->events.on_channel_create));
}

uint8_t on_channel_update(Client *client, const cJSON *d)
{
    return (channel_put(client, d, NULL, client->events.on_channel_update));
}

uint8_t on_channel_delete(Client *client, const cJSON *d)
{
    return (channel_remove(client, d, client->events.on_channel_delete));
}

uint8_t on_thread_create(Client *client, const cJSON *d)
{
    return (channel_put(client, d, NULL, client->events.on_thread_create));
}

uint8_t on_thread_update(Client *client, const cJSON *d)
{
    return (channel_put(client, d, NULL, client->events.on_thread_update));
}

uint8_t on_thread_delete(Client *client, const cJSON *d)
{
    return (channel_remove(client, d, client->events.on_thread_delete));
}

uint8_t on_thread_list_sync(Client *client, const cJSON *d)
{
    const cJSON *threads = cJSON_GetObjectItemCaseSensitive(d, "threads");
    const cJSON *item;

    if (!json_str(d, "guild_id") || !cJSON_IsArray(threads)) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    cJSON_ArrayForEach(item, threads) {
        if (channel_put(client, item, json_str(d, "guild_id"), NULL)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    return (0);
}

uint8_t on_channel_pins_update(Client *client, const cJSON *d)
{
    Channel *channel;
    char *timestamp = NULL;

    if (!json_str(d, "channel_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (json_str(d, "last_pin_timestamp")) {
        timestamp = strdup(json_str(d, "last_pin_timestamp"));

        if (!timestamp) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new last_pin_timestamp.");
            return (1);
        }
    }

    client_cache_lock(client);

    channel = find_channel_unlocked(client, json_str(d, "guild_id"), json_str(d, "channel_id"));
    if (channel) {
        free(channel->last_pin_timestamp);
        channel->last_pin_timestamp = timestamp;
    }

    client_cache_unlock(client);

    if (!channel) {
        free(timestamp);
        LOG_W("channel %s not found, skipping pins update.", json_str(d, "channel_id"));
        return (0);
    }

    FIRE(client, on_channel_pins_update, channel);
    return (0);
}

uint8_t on_webhooks_update(Client *client, const cJSON *d)
{
    if (!json_str(d, "guild_id") || !json_str(d, "channel_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    FIRE(client, on_webhooks_update, json_str(d, "guild_id"), json_str(d, "channel_id"));
    return (0);
}
