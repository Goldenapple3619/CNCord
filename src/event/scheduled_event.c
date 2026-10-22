
#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

static uint8_t scheduled_event(Client *client, const cJSON *d, uint8_t (*cb)(Client *, const ScheduledEvent *))
{
    ScheduledEvent event = {0};

    event.id = json_str(d, "id");
    event.guild_id = json_str(d, "guild_id");
    event.channel_id = json_str(d, "channel_id");
    event.creator_id = json_str(d, "creator_id");
    event.name = json_str(d, "name");
    event.description = json_str(d, "description");
    event.start_time = json_str(d, "scheduled_start_time");
    event.end_time = json_str(d, "scheduled_end_time");
    event.status = json_int(d, "status");
    event.entity_type = json_int(d, "entity_type");

    if (!event.id || !event.guild_id || !event.name) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    LOG_D("<SCHEDULED EVENT id='%s' name='%s'>", event.id, event.name);

    if (cb && cb(client, &event)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_scheduled_event_create(Client *client, const cJSON *d)
{
    return (scheduled_event(client, d, client->events.on_scheduled_event_create));
}

uint8_t on_scheduled_event_update(Client *client, const cJSON *d)
{
    return (scheduled_event(client, d, client->events.on_scheduled_event_update));
}

uint8_t on_scheduled_event_delete(Client *client, const cJSON *d)
{
    return (scheduled_event(client, d, client->events.on_scheduled_event_delete));
}

uint8_t on_scheduled_event_user_add(Client *client, const cJSON *d)
{
    if (!json_str(d, "guild_id") || !json_str(d, "guild_scheduled_event_id") || !json_str(d, "user_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    FIRE(client, on_scheduled_event_user_add, json_str(d, "guild_id"), json_str(d, "guild_scheduled_event_id"), json_str(d, "user_id"));
    return (0);
}

uint8_t on_scheduled_event_user_remove(Client *client, const cJSON *d)
{
    if (!json_str(d, "guild_id") || !json_str(d, "guild_scheduled_event_id") || !json_str(d, "user_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    FIRE(client, on_scheduled_event_user_remove, json_str(d, "guild_id"), json_str(d, "guild_scheduled_event_id"), json_str(d, "user_id"));
    return (0);
}
