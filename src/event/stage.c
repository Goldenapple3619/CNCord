#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

static uint8_t stage_event(Client *client, const cJSON *d, uint8_t (*cb)(Client *, const StageInstance *))
{
    StageInstance stage = {0};

    stage.id = json_str(d, "id");
    stage.guild_id = json_str(d, "guild_id");
    stage.channel_id = json_str(d, "channel_id");
    stage.topic = json_str(d, "topic");
    stage.privacy_level = json_int(d, "privacy_level");

    if (!stage.id || !stage.guild_id || !stage.channel_id) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    LOG_D("<STAGE id='%s' channel_id='%s'>", stage.id, stage.channel_id);

    if (cb && cb(client, &stage)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_stage_create(Client *client, const cJSON *d)
{
    return (stage_event(client, d, client->events.on_stage_create));
}

uint8_t on_stage_update(Client *client, const cJSON *d)
{
    return (stage_event(client, d, client->events.on_stage_update));
}

uint8_t on_stage_delete(Client *client, const cJSON *d)
{
    return (stage_event(client, d, client->events.on_stage_delete));
}