#include "client.h"
#include "api.h"
#include "log.h"
#include "error.h"

CN_API uint8_t client_api_delete_channel(Client *client, const char *channel_id)
{
    if (client_api_call(client, "DELETE", NULL, NULL, "/channels/%s", channel_id)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}
