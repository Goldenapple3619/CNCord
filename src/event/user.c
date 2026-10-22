#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

uint8_t on_typing_start(Client *client, const cJSON *d)
{
    if (!json_str(d, "channel_id") || !json_str(d, "user_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    FIRE(client, on_typing_start, json_str(d, "guild_id"), json_str(d, "channel_id"), json_str(d, "user_id"));
    return (0);
}

uint8_t on_presence_update(Client *client, const cJSON *d)
{
    char *user_id = json_str(cJSON_GetObjectItemCaseSensitive(d, "user"), "id");

    if (!user_id) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    FIRE(client, on_presence_update, json_str(d, "guild_id"), user_id, json_str(d, "status"));
    return (0);
}

uint8_t on_user_update(Client *client, const cJSON *d)
{
    User user = {0};

    if (parse_user(&user, d, NULL)) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<USER UPDATE id='%s' username='%s'>", user.id, user.username ? user.username : "?");

    FIRE(client, on_user_update, &user);
    return (0);
}
