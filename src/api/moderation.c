#include "api.h"
#include "log.h"
#include "error.h"

#include <cJSON.h>

CN_API uint8_t client_api_kick_member(Client *client, const char *guild_id, const char *user_id)
{
    if (client_api_call(client, "DELETE", NULL, NULL, "/guilds/%s/members/%s", guild_id, user_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API uint8_t client_api_ban_member(Client *client, const char *guild_id, const char *user_id, uint32_t delete_message_seconds)
{
    cJSON *body = cJSON_CreateObject();

    if (delete_message_seconds > 604800)
        delete_message_seconds = 604800;

    if (!body || !cJSON_AddNumberToObject(body, "delete_message_seconds", (double)delete_message_seconds)) {
        cJSON_Delete(body);
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate ban body.");
        return (1);
    }

    if (client_api_call(client, "PUT", body, NULL, "/guilds/%s/bans/%s", guild_id, user_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API uint8_t client_api_unban_member(Client *client, const char *guild_id, const char *user_id)
{
    if (client_api_call(client, "DELETE", NULL, NULL, "/guilds/%s/bans/%s", guild_id, user_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API uint8_t client_api_add_role(Client *client, const char *guild_id, const char *user_id, const char *role_id)
{
    if (client_api_call(client, "PUT", NULL, NULL, "/guilds/%s/members/%s/roles/%s", guild_id, user_id, role_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API uint8_t client_api_remove_role(Client *client, const char *guild_id, const char *user_id, const char *role_id)
{
    if (client_api_call(client, "DELETE", NULL, NULL, "/guilds/%s/members/%s/roles/%s", guild_id, user_id, role_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}