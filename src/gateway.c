#include "client.h"
#include "error.h"
#include "clock.h"
#include "log.h"

#include <cJSON.h>

CN_API uint8_t client_gw_send(Client *client, cJSON *root)
{
    uint8_t status = 0;

    char *json = cJSON_PrintUnformatted(root);

    if (!json) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new json str.");
        return (1);
    }
    if (client_socket_send_text(client, json)) {
        PROPAGATE_ERR();
        status = 1;
    }
    free(json);
    return (status);
}

CN_API uint8_t client_gw_send_heartbeat(Client *client)
{
    uint8_t status;
    cJSON *root = cJSON_CreateObject();

    cJSON_AddNumberToObject(root, "op", 1);
    if (client->session.seq >= 0)
        cJSON_AddNumberToObject(root, "d", (double)client->session.seq);
    else
        cJSON_AddNullToObject(root, "d");
    status = client_gw_send(client, root);
    cJSON_Delete(root);

    if (status) {
        PROPAGATE_ERR();
    }
    return (status);
}

CN_API uint8_t client_gw_send_identify(Client *client)
{
    uint8_t status;
    cJSON *root  = cJSON_CreateObject();
    cJSON *props;
    cJSON *d;

    cJSON_AddNumberToObject(root, "op", 2);
    d = cJSON_AddObjectToObject(root, "d");
    cJSON_AddStringToObject(d, "token", client->token);
    cJSON_AddNumberToObject(d, "intents", client->intents);
    props = cJSON_AddObjectToObject(d, "properties");
    cJSON_AddStringToObject(props, "os",
        #if defined(_WIN32)
                "windows"
        #elif defined(__APPLE__)
                "macos"
        #elif defined(__linux__)
                "linux"
        #else
                "other"
        #endif
    );
    cJSON_AddStringToObject(props, "browser", "c-discord");
    cJSON_AddStringToObject(props, "device", "c-discord");

    status = client_gw_send(client, root);
    cJSON_Delete(root);

    if (status) {
        PROPAGATE_ERR();
    }
    return (status);
}

CN_API uint8_t client_gw_send_resume(Client *client)
{
    uint8_t status;
    cJSON *root = cJSON_CreateObject();
    cJSON *d;

    cJSON_AddNumberToObject(root, "op", 6);
    d = cJSON_AddObjectToObject(root, "d");
    cJSON_AddStringToObject(d, "token", client->token);
    cJSON_AddStringToObject(d, "session_id", client->session.id);
    cJSON_AddNumberToObject(d, "seq", (double)client->session.seq);
    status = client_gw_send(client, root);
    cJSON_Delete(root);

    if (status) {
        PROPAGATE_ERR();
    }
    return (status);
}

CN_API Action client_gw_handle(Client *client, const char *json)
{
    cJSON *root = cJSON_Parse(json);

    if (!root) {
        RAISE(ERR_CORRUPT_OR_INVALID, "bad json received.");
        return ACT_NONE;
    }

    const cJSON *op = cJSON_GetObjectItemCaseSensitive(root, "op");
    const cJSON *s  = cJSON_GetObjectItemCaseSensitive(root, "s");
    const cJSON *t  = cJSON_GetObjectItemCaseSensitive(root, "t");
    const cJSON *d  = cJSON_GetObjectItemCaseSensitive(root, "d");
    Action act = ACT_NONE;

    if (cJSON_IsNumber(s))
        client->session.seq = (long long)s->valuedouble;

    switch (cJSON_IsNumber(op) ? op->valueint : -1) {
    case 0:  /* Dispatch */
        if (cJSON_IsString(t))
            if (handle_dispatch(client, t->valuestring, d)) {
                PROPAGATE_ERR();
                cJSON_Delete(root);
                return (ACT_NONE);
            }
        break;

    case 1:  /* server asks for an immediate heartbeat */
        client_gw_send_heartbeat(client);
        break;

    case 7:  /* Reconnect */
        LOG_W("server requested reconnect");
        act = ACT_RECONNECT;
        break;

    case 9:  /* Invalid session; d = true if resumable */
        LOG_W("invalid session");
        act = cJSON_IsTrue(d) ? ACT_RECONNECT : ACT_NEW_SESSION;
        break;

    case 10: { /* Hello */
        const cJSON *hb = cJSON_IsObject(d)
            ? cJSON_GetObjectItemCaseSensitive(d, "heartbeat_interval") : NULL;
        client->session.heartbeat_ms = cJSON_IsNumber(hb) ? (long long)hb->valuedouble : 41250;

        double jitter = (double)rand() / (double)RAND_MAX;
        client->session.next_heartbeat = now_ms() + (long long)((double)client->session.heartbeat_ms * jitter);
        client->session.ack_pending = false;

        if (client->session.id && client->session.seq >= 0) {
            LOG_I("resuming session %s.", client->session.id ? client->session.id : "?");
            client_gw_send_resume(client);
        } else {
            LOG_I("identifying with intents %" PRIu64, client->intents);
            client_gw_send_identify(client);
        }
        break;
    }

    case 11: /* Heartbeat ACK */
        client->session.ack_pending = false;
        break;

    default:
        break;
    }

    cJSON_Delete(root);
    return act;
}