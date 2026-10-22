#include "api.h"
#include "log.h"
#include "error.h"

#include <curl/curl.h>

CN_API uint8_t client_api_add_reaction(Client *client, const char *channel_id, const char *message_id, const char *emoji)
{
    char *escaped = curl_easy_escape(client->connection.http, emoji, 0);
    uint8_t status;

    if (!escaped) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to escape emoji.");
        return (1);
    }

    status = client_api_call(client, "PUT", NULL, NULL, "/channels/%s/messages/%s/reactions/%s/@me", channel_id, message_id, escaped);
    curl_free(escaped);

    if (status) {
        PROPAGATE_ERR();
    }
    return (status);
}
