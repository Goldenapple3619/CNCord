#include "api.h"
#include "log.h"
#include "error.h"

CN_API uint8_t client_api_trigger_typing(Client *client, const char *channel_id)
{
    if (client_api_call(client, "POST", NULL, NULL, "/channels/%s/typing", channel_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}
