#include "client.h"
#include "api.h"
#include "log.h"
#include "error.h"

CN_API uint8_t client_api_leave_guild(Client *client, const char *guild_id)
{
    if (client_api_call(client, "DELETE", NULL, NULL, "/users/@me/guilds/%s", guild_id)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}
