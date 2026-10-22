#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

static uint8_t invite_event(Client *client, const cJSON *d, uint8_t (*cb)(Client *, const Invite *))
{
    Invite invite = {0};

    invite.code = json_str(d, "code");
    invite.channel_id = json_str(d, "channel_id");
    invite.guild_id = json_str(d, "guild_id");
    invite.max_age = json_u64(d, "max_age");
    invite.max_uses = json_u64(d, "max_uses");
    invite.temporary = json_flag(d, "temporary");

    if (!invite.code || !invite.channel_id) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    LOG_D("<INVITE code='%s' channel_id='%s'>", invite.code, invite.channel_id);

    if (cb && cb(client, &invite)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t on_invite_create(Client *client, const cJSON *d)
{
    return (invite_event(client, d, client->events.on_invite_create));
}

uint8_t on_invite_delete(Client *client, const cJSON *d)
{
    return (invite_event(client, d, client->events.on_invite_delete));
}