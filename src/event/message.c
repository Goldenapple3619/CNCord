#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

uint8_t on_message_create(Client *client, const cJSON *d)
{
    Message msg = {0};
    Message *copy;
    Message *dropped = NULL;
    Channel *channel;
    uint8_t status = 0;
    cnbool cached = false;
    cnbool failed = false;

    if (!cJSON_IsString(cJSON_GetObjectItemCaseSensitive(d, "content"))
        || !cJSON_IsObject(cJSON_GetObjectItemCaseSensitive(d, "author"))) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (parse_message(&msg, d)) {
        PROPAGATE_ERR();
        return (1);
    }

    copy = message_dup(&msg);
    empty_generic_vector(&msg.mentions, (void (*)(void *))&user_dedup);
    empty_generic_vector(&msg.mention_roles, &free);
    empty_generic_vector(&msg.attachments, (void (*)(void *))&attachment_dedup);
    empty_generic_vector(&msg.embeds, (void (*)(void *))&embed_dedup);

    if (!copy) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<MESSAGE content='%s', author=<AUTHOR id='%s' username='%s'>>",
        copy->content ? copy->content : "?",
        copy->author.id ? copy->author.id : "?",
        copy->author.username ? copy->author.username : "?");

    client_cache_lock(client);

    channel = copy->guild_id
        ? find_guild_channel_unlocked(client, copy->guild_id, copy->channel_id)
        : get_dm_channel_unlocked(client, copy);

    if (channel) {
        if (add_message_unlocked(channel, copy, &dropped))
            failed = true;
        else
            cached = true;
    } else if (!copy->guild_id) {
        failed = true;
    }

    client_cache_unlock(client);

    if (dropped)
        message_dedup(dropped);

    if (failed) {
        message_dedup(copy);
        RAISE(ERR_OUT_OF_MEMORY, "failed to cache message.");
        return (1);
    }

    if (!channel)
        LOG_W("no channel %s found in guild %s, skipping message caching.", copy->channel_id, copy->guild_id);

    if (client->events.on_message_create)
        status = client->events.on_message_create(client, copy);

    if (!cached)
        message_dedup(copy);

    if (status) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_message_update(Client *client, const cJSON *d)
{
    Message msg = {0};
    Message *copy;
    Message *old = NULL;
    Channel *channel;
    size_t i = NOT_FOUND;
    uint8_t status = 0;

    if (!json_str(d, "id") || !json_str(d, "channel_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (!cJSON_IsString(cJSON_GetObjectItemCaseSensitive(d, "content"))
        || !cJSON_IsObject(cJSON_GetObjectItemCaseSensitive(d, "author")))
        return (0);

    if (parse_message(&msg, d)) {
        PROPAGATE_ERR();
        return (1);
    }

    copy = message_dup(&msg);
    empty_generic_vector(&msg.mentions, (void (*)(void *))&user_dedup);
    empty_generic_vector(&msg.mention_roles, &free);
    empty_generic_vector(&msg.attachments, (void (*)(void *))&attachment_dedup);
    empty_generic_vector(&msg.embeds, (void (*)(void *))&embed_dedup);

    if (!copy) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<MESSAGE UPDATE id='%s' content='%s'>", copy->id, copy->content ? copy->content : "?");

    client_cache_lock(client);

    channel = find_channel_unlocked(client, copy->guild_id, copy->channel_id);
    if (channel)
        i = find_by_id(&channel->messages, copy->id);

    if (i != NOT_FOUND) {
        old = channel->messages.content[i];
        channel->messages.content[i] = copy;
    }

    client_cache_unlock(client);

    if (old)
        message_dedup(old);

    if (client->events.on_message_update)
        status = client->events.on_message_update(client, copy);

    if (i == NOT_FOUND)
        message_dedup(copy);

    if (status) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

static uint8_t delete_message(Client *client, char *id, char *channel_id, char *guild_id)
{
    Message msg = {0};
    Message *old = NULL;
    Channel *channel;
    uint8_t status = 0;

    msg.id = id;
    msg.channel_id = channel_id;
    msg.guild_id = guild_id;

    LOG_D("~<MESSAGE id='%s' channel_id='%s'>", id, channel_id);

    client_cache_lock(client);

    channel = find_channel_unlocked(client, guild_id, channel_id);
    if (channel)
        old = take_by_id(&channel->messages, id);

    client_cache_unlock(client);

    if (client->events.on_message_delete)
        status = client->events.on_message_delete(client, old ? old : &msg);

    if (old)
        message_dedup(old);

    if (status) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_message_delete(Client *client, const cJSON *d)
{
    if (!json_str(d, "id") || !json_str(d, "channel_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    return (delete_message(client, json_str(d, "id"), json_str(d, "channel_id"), json_str(d, "guild_id")));
}

uint8_t on_message_delete_bulk(Client *client, const cJSON *d)
{
    const cJSON *ids = cJSON_GetObjectItemCaseSensitive(d, "ids");
    const cJSON *item;
    uint8_t status = 0;

    if (!cJSON_IsArray(ids) || !json_str(d, "channel_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    cJSON_ArrayForEach(item, ids) {
        if (!cJSON_IsString(item)) {
            RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
            return (1);
        }

        if (delete_message(client, item->valuestring, json_str(d, "channel_id"), json_str(d, "guild_id")))
            status = 1;
    }

    return (status);
}
