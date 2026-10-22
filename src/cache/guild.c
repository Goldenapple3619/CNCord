#include "client.h"
#include "cache.h"

void swap_guild_info_unlocked(Guild *cached, Guild *fresh)
{
    swap_str(&cached->name, &fresh->name);
    swap_str(&cached->icon, &fresh->icon);
    swap_str(&cached->owner_id, &fresh->owner_id);
    swap_str(&cached->description, &fresh->description);
    swap_vec(&cached->roles, &fresh->roles);
    swap_vec(&cached->emojis, &fresh->emojis);
}

Guild *find_guild_unlocked(Client *client, const char *id)
{
    size_t i = find_by_id(&client->cache.guilds, id);

    return (i == NOT_FOUND ? NULL : client->cache.guilds.content[i]);
}

uint8_t put_guild_unlocked(Client *client, Guild *fresh, Guild **old)
{
    for (size_t i = 0; i < client->cache.guilds.size; ++i) {
        Guild *cached = client->cache.guilds.content[i];

        if (strcmp(cached->id, fresh->id))
            continue;

        for (size_t j = 0; j < fresh->channels.size; ++j) {
            Channel *new_channel = fresh->channels.content[j];
            Channel *old_channel = find_channel_in(&cached->channels, new_channel->id);

            if (old_channel)
                swap_vec(&new_channel->messages, &old_channel->messages);
        }

        client->cache.guilds.content[i] = fresh;
        *old = cached;
        return (0);
    }

    return (insert_generic_vector(&client->cache.guilds, fresh));
}
