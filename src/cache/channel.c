#include "client.h"
#include "cache.h"

Channel *find_guild_channel_unlocked(Client *client, const char *guild_id, const char *channel_id)
{
    Guild *guild = find_guild_unlocked(client, guild_id);

    return (guild ? find_channel_in(&guild->channels, channel_id) : NULL);
}

Channel *find_channel_unlocked(Client *client, const char *guild_id, const char *channel_id)
{
    if (guild_id)
        return (find_guild_channel_unlocked(client, guild_id, channel_id));
    return (find_channel_in(&client->cache.dm_channels, channel_id));
}

Channel *get_dm_channel_unlocked(Client *client, const Message *msg)
{
    cnbool from_us = (msg->author.id && client->id && !strcmp(msg->author.id, client->id));
    char *name = from_us ? NULL : msg->author.username;
    Channel *channel = find_channel_in(&client->cache.dm_channels, msg->channel_id);
    Channel fresh;

    if (channel) {
        if (!channel->name && name)
            channel->name = strdup(name);
        return (channel);
    }

    memset(&fresh, 0, sizeof(fresh));
    fresh.id = msg->channel_id;
    fresh.name = name;
    fresh.type = CHAN_DM;

    channel = channel_dup(&fresh);
    if (!channel)
        return (NULL);

    if (insert_generic_vector(&client->cache.dm_channels, channel)) {
        channel_dedup(channel);
        return (NULL);
    }
    return (channel);
}

uint8_t remove_channel_unlocked(Client *client, const char *guild_id, const char *channel_id, Channel **old)
{
    Guild *found = NULL;
    struct generic_vector_s *lookup;

    if (guild_id) {
        found = find_guild_unlocked(client, guild_id);

        if (!found) {
            RAISE_FMT(ERR_OUT_OF_BOUND, "failed to remove channel in guild, guild %s not found.", guild_id);
            return (1);
        }
    }

    lookup = found ? &found->channels : &client->cache.dm_channels;

    for (size_t i = 0; i < lookup->size; ++i) {
        Channel *cached = lookup->content[i];

        if (strcmp(cached->id, channel_id))
            continue;

        remove_generic_ordered_vector(lookup, i, NULL);

        *old = cached;
        return (0);
    }

    RAISE_FMT(ERR_OUT_OF_BOUND, "failed to remove channel, %s not found.", channel_id);
    return (1);
}

uint8_t put_channel_unlocked(Client *client, const char *guild_id, Channel *fresh, Channel **old)
{
    Guild *found = NULL;
    struct generic_vector_s *lookup;

    if (guild_id) {
        found = find_guild_unlocked(client, guild_id);

        if (!found) {
            RAISE_FMT(ERR_OUT_OF_BOUND, "failed to add channel to guild, guild %s not found.", guild_id);
            return (1);
        }
    }

    lookup = found ? &found->channels : &client->cache.dm_channels;

    for (size_t i = 0; i < lookup->size; ++i) {
        Channel *cached = lookup->content[i];

        if (strcmp(cached->id, fresh->id))
            continue;

        swap_vec(&fresh->messages, &cached->messages);

        lookup->content[i] = fresh;
        *old = cached;
        return (0);
    }

    return (insert_generic_vector(lookup, fresh));
}
