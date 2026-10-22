#include "api.h"
#include "log.h"
#include "error.h"
#include <cJSON.h>

static uint8_t interaction_callback(Client *client, const Interaction *it, int type, const char *content,
    const Embed *embeds, size_t embed_count, cnbool ephemeral)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *data = NULL;

    if (!root || !cJSON_AddNumberToObject(root, "type", type)) {
        cJSON_Delete(root);
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate interaction response.");
        return (1);
    }

    if (content || embed_count) {
        data = new_message_body(content);

        if (!data) {
            cJSON_Delete(root);
            PROPAGATE_ERR();
            return (1);
        }

        if (!cJSON_AddItemToObject(root, "data", data)) {
            cJSON_Delete(data);
            cJSON_Delete(root);
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate interaction response.");
            return (1);
        }

        if (message_body_add_embeds(data, embeds, embed_count)) {
            cJSON_Delete(root);
            PROPAGATE_ERR();
            return (1);
        }
    } else if (ephemeral) {
        data = cJSON_AddObjectToObject(root, "data");
    }

    if (ephemeral && (!data || !cJSON_AddNumberToObject(data, "flags", MSG_FLAG_EPHEMERAL))) {
        cJSON_Delete(root);
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate interaction response.");
        return (1);
    }

    if (client_api_call(client, "POST", root, NULL, "/interactions/%s/%s/callback", it->id, it->token)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

static uint8_t interaction_edit(Client *client, const Interaction *it, const char *content, const Embed *embeds, size_t embed_count)
{
    cJSON *body = new_message_body(content);

    if (!body) {
        PROPAGATE_ERR();
        return (1);
    }

    if (message_body_add_embeds(body, embeds, embed_count)) {
        cJSON_Delete(body);
        PROPAGATE_ERR();
        return (1);
    }

    if (client_api_call(client, "PATCH", body, NULL, "/webhooks/%s/%s/messages/@original", it->application_id, it->token)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

static uint8_t interaction_followup(Client *client, const Interaction *it, const char *content,
    const Embed *embeds, size_t embed_count, cnbool ephemeral)
{
    cJSON *body = new_message_body(content);

    if (!body) {
        PROPAGATE_ERR();
        return (1);
    }

    if (message_body_add_embeds(body, embeds, embed_count)) {
        cJSON_Delete(body);
        PROPAGATE_ERR();
        return (1);
    }

    if (ephemeral && !cJSON_AddNumberToObject(body, "flags", MSG_FLAG_EPHEMERAL)) {
        cJSON_Delete(body);
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate followup body.");
        return (1);
    }

    if (client_api_call(client, "POST", body, NULL, "/webhooks/%s/%s", it->application_id, it->token)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API uint8_t client_interaction_reply(Client *client, const Interaction *interaction, const char *content, cnbool ephemeral)
{
    return (interaction_callback(client, interaction, 4, content, NULL, 0, ephemeral));   /* 4 = CHANNEL_MESSAGE_WITH_SOURCE */
}

CN_API uint8_t client_interaction_reply_embeds(Client *client, const Interaction *interaction, const char *content,
    const Embed *embeds, size_t count, cnbool ephemeral)
{
    return (interaction_callback(client, interaction, 4, content, embeds, count, ephemeral));
}

CN_API uint8_t client_interaction_defer(Client *client, const Interaction *interaction, cnbool ephemeral)
{
    return (interaction_callback(client, interaction, 5, NULL, NULL, 0, ephemeral));      /* 5 = DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE */
}

CN_API uint8_t client_interaction_edit_reply(Client *client, const Interaction *interaction, const char *content)
{
    return (interaction_edit(client, interaction, content, NULL, 0));
}

CN_API uint8_t client_interaction_edit_reply_embeds(Client *client, const Interaction *interaction, const char *content,
    const Embed *embeds, size_t count)
{
    return (interaction_edit(client, interaction, content, embeds, count));
}

CN_API uint8_t client_interaction_followup(Client *client, const Interaction *interaction, const char *content, cnbool ephemeral)
{
    return (interaction_followup(client, interaction, content, NULL, 0, ephemeral));
}

CN_API uint8_t client_interaction_followup_embeds(Client *client, const Interaction *interaction, const char *content,
    const Embed *embeds, size_t count, cnbool ephemeral)
{
    return (interaction_followup(client, interaction, content, embeds, count, ephemeral));
}