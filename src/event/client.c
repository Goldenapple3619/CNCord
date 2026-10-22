#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

uint8_t on_ready(Client *client, const cJSON *d)
{
    const cJSON *user = cJSON_GetObjectItemCaseSensitive(d, "user");
    const cJSON *app = cJSON_GetObjectItemCaseSensitive(d, "application");

    if (client_set_session(client,
        json_str(user, "id"),
        json_str(user, "username"),
        json_str(d, "resume_gateway_url"),
        json_str(d, "session_id"))) {
            PROPAGATE_ERR();
            return (1);
    }

    LOG_D("<READY username='%s' id='%s'>",
        client->username ? client->username : "?",
        client->id ? client->id : "?");

    if (client->application_id)
        free(client->application_id);
    client->application_id = NULL;

    if (json_str(app, "id")) {
        client->application_id = strdup(json_str(app, "id"));

        if (!client->application_id) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate application id.");
            return (1);
        }
    }

    if (client->commands.size && client_sync_commands(client, NULL)) {
        PROPAGATE_ERR();
        return (1);
    }

    if (client->events.on_ready) {
        if (client->events.on_ready(client)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    return (0);
}

uint8_t on_resume(Client *client, const cJSON *d)
{
    (void)client;
    (void)d;

    LOG_D("<RESUME>");

    if (client->events.on_resume) {
        if (client->events.on_resume(client)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    return (0);
}

uint8_t on_error(Client *client, const ErrorContext *ctx)
{
    (void)client;

    LOG_D("<ERROR>");

    LOG_E("[%s] %s", error_type_to_text(ctx->code), ctx->message);
    for (size_t i = 0; i < ctx->frame_count; ++i) {
        if (i != (ctx->frame_count - 1))
            LOG_E("  |-[%zu] %s@%s:%" PRIu32 "", ctx->frame_count - i, ctx->frames[i].function ? ctx->frames[i].function : "???", ctx->frames[i].file ? ctx->frames[i].file : "???", ctx->frames[i].line);
        else
            LOG_E("  \\-[%zu] %s@%s:%" PRIu32 "", ctx->frame_count - i, ctx->frames[i].function ? ctx->frames[i].function : "???", ctx->frames[i].file ? ctx->frames[i].file : "???", ctx->frames[i].line);
    }

    if (client->events.on_error) {
        if (client->events.on_error(client, ctx)) {
            PROPAGATE_ERR();
            return (1);
        }
    }

    return (0);
}