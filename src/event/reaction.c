#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

static uint8_t reaction_event(Client *client, const cJSON *d,
    uint8_t (*cb)(Client *, const Reaction *), cnbool need_user, cnbool need_emoji)
{
    Reaction reaction = {0};
    const cJSON *emoji = cJSON_GetObjectItemCaseSensitive(d, "emoji");

    reaction.user_id = json_str(d, "user_id");
    reaction.message_id = json_str(d, "message_id");
    reaction.channel_id = json_str(d, "channel_id");
    reaction.guild_id = json_str(d, "guild_id");

    if (cJSON_IsObject(emoji)) {
        reaction.emoji.id = json_str(emoji, "id");
        reaction.emoji.name = json_str(emoji, "name");
        reaction.emoji.animated = json_flag(emoji, "animated");
    }

    if (!reaction.message_id || !reaction.channel_id
        || (need_user && !reaction.user_id) || (need_emoji && !cJSON_IsObject(emoji))) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    LOG_D("<REACTION message_id='%s' emoji='%s'>", reaction.message_id, reaction.emoji.name ? reaction.emoji.name : "?");

    if (cb && cb(client, &reaction)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_reaction_add(Client *client, const cJSON *d)
{
    return (reaction_event(client, d, client->events.on_reaction_add, true, true));
}

uint8_t on_reaction_remove(Client *client, const cJSON *d)
{
    return (reaction_event(client, d, client->events.on_reaction_remove, true, true));
}

uint8_t on_reaction_remove_all(Client *client, const cJSON *d)
{
    return (reaction_event(client, d, client->events.on_reaction_remove_all, false, false));
}

uint8_t on_reaction_remove_emoji(Client *client, const cJSON *d)
{
    return (reaction_event(client, d, client->events.on_reaction_remove_emoji, false, true));
}