#include "api.h"
#include "buffer.h"
#include "log.h"

#include <cJSON.h>

static size_t utf16_prefix(const char *s, size_t max_units)
{
    size_t i = 0;
    size_t units = 0;
    size_t len;
    size_t k;
    unsigned char c;

    while (s[i]) {
        c = (unsigned char)s[i];
        len = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;

        for (k = 1; k < len; k++) {
            if (!s[i + k]) {
                len = k;
                break;
            }
        }

        if (units + (len == 4 ? 2 : 1) > max_units)
            break;

        units += len == 4 ? 2 : 1;
        i += len;
    }
    return (i);
}

static cnbool put_str(cJSON *obj, const char *key, const char *s, size_t max_units)
{
    char *tmp;
    cnbool ok;

    if (!s || !*s)
        return (true);

    tmp = strndup(s, utf16_prefix(s, max_units));

    if (!tmp)
        return (false);

    ok = cJSON_AddStringToObject(obj, key, tmp) != NULL;
    free(tmp);
    return (ok);
}

static cnbool add_media(cJSON *obj, const char *key, const char *url)
{
    cJSON *sub;

    if (!url || !*url)
        return (true);

    sub = cJSON_AddObjectToObject(obj, key);
    return (sub && put_str(sub, "url", url, EMBED_URL_MAX));
}

static cnbool add_footer(cJSON *obj, const Embed *embed)
{
    cJSON *sub;

    if (!embed->footer_text || !*embed->footer_text)
        return (true);

    sub = cJSON_AddObjectToObject(obj, "footer");
    return (sub
        && put_str(sub, "text", embed->footer_text, EMBED_FOOTER_MAX)
        && put_str(sub, "icon_url", embed->footer_icon_url, EMBED_URL_MAX));
}

static cnbool add_author(cJSON *obj, const Embed *embed)
{
    cJSON *sub;

    if (!embed->author_name || !*embed->author_name)
        return (true);

    sub = cJSON_AddObjectToObject(obj, "author");
    return (sub
        && put_str(sub, "name", embed->author_name, EMBED_AUTHOR_MAX)
        && put_str(sub, "url", embed->author_url, EMBED_URL_MAX)
        && put_str(sub, "icon_url", embed->author_icon_url, EMBED_URL_MAX));
}

static cnbool add_fields(cJSON *obj, const Embed *embed)
{
    const EmbedField *field;
    cJSON *array;
    cJSON *item;
    size_t i;

    if (!embed->fields.size)
        return (true);

    array = cJSON_AddArrayToObject(obj, "fields");

    if (!array)
        return (false);

    for (i = 0; i < embed->fields.size && i < EMBED_MAX_FIELDS; ++i) {
        field = embed->fields.content[i];
        item = cJSON_CreateObject();

        if (!item)
            return (false);

        cJSON_AddItemToArray(array, item);

        if (!put_str(item, "name", field->name && *field->name ? field->name : ZERO_WIDTH_SPACE, EMBED_FIELD_NAME_MAX)
            || !put_str(item, "value", field->value && *field->value ? field->value : ZERO_WIDTH_SPACE, EMBED_FIELD_VALUE_MAX)
            || !cJSON_AddBoolToObject(item, "inline", field->is_inline ? 1 : 0))
            return (false);
    }
    return (true);
}

cJSON *embed_to_json(const Embed *embed)
{
    cJSON *obj = cJSON_CreateObject();

    if (obj
        && put_str(obj, "title", embed->title, EMBED_TITLE_MAX)
        && put_str(obj, "description", embed->description, EMBED_DESCRIPTION_MAX)
        && put_str(obj, "url", embed->url, EMBED_URL_MAX)
        && put_str(obj, "timestamp", embed->timestamp, EMBED_URL_MAX)
        && (!embed->has_color || cJSON_AddNumberToObject(obj, "color", (double)(embed->color & 0xFFFFFFu)))
        && add_footer(obj, embed)
        && add_media(obj, "image", embed->image_url)
        && add_media(obj, "thumbnail", embed->thumbnail_url)
        && add_author(obj, embed)
        && add_fields(obj, embed))
        return (obj);

    cJSON_Delete(obj);
    RAISE(ERR_OUT_OF_MEMORY, "failed to allocate embed.");
    return (NULL);
}

uint8_t message_body_add_embeds(cJSON *body, const Embed *embeds, size_t count)
{
    cJSON *array;
    cJSON *item;
    size_t i;

    if (!count)
        return (0);

    if (!embeds || count > EMBED_MAX_PER_MESSAGE) {
        RAISE(ERR_OUT_OF_BOUND, "a message can carry at most 10 embeds.");
        return (1);
    }

    array = cJSON_AddArrayToObject(body, "embeds");

    if (!array) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate message embeds.");
        return (1);
    }

    for (i = 0; i < count; ++i) {
        item = embed_to_json(&embeds[i]);

        if (!item) {
            PROPAGATE_ERR();
            return (1);
        }
        cJSON_AddItemToArray(array, item);
    }
    return (0);
}

cJSON *new_message_body(const char *content)
{
    size_t n = content ? strlen(content) : 0;
    char *text = NULL;
    cJSON *body;
    cJSON *am;

    if (n > 2000) {
        n = 2000;

        while (n > 0 && ((unsigned char)content[n] & 0xC0) == 0x80)
            n--;
    }

    if (content) {
        text = strndup(content, n);

        if (!text) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new content for message.");
            return (NULL);
        }
    }

    body = cJSON_CreateObject();

    if (!body) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate message body.");
        free(text);
        return (NULL);
    }

    am = cJSON_AddObjectToObject(body, "allowed_mentions");

    if (!am || !cJSON_AddArrayToObject(am, "parse") || (text && !cJSON_AddStringToObject(body, "content", text))) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate message body.");
        cJSON_Delete(body);
        free(text);
        return (NULL);
    }

    free(text);
    return (body);
}

static uint8_t send_message(Client *client, const char *channel_id, const char *content, const char *reply_to,
    const Embed *embeds, size_t embed_count)
{
    cJSON *body;
    cJSON *ref;

    if (!content && !embed_count) {
        RAISE(ERR_CORRUPT_OR_INVALID, "a message needs content or at least one embed.");
        return (1);
    }

    body = new_message_body(content);

    if (!body) {
        PROPAGATE_ERR();
        return (1);
    }

    if (message_body_add_embeds(body, embeds, embed_count)) {
        cJSON_Delete(body);
        PROPAGATE_ERR();
        return (1);
    }

    if (reply_to) {
        ref = cJSON_AddObjectToObject(body, "message_reference");

        if (ref) {
            cJSON_AddStringToObject(ref, "message_id", reply_to);
            cJSON_AddBoolToObject(ref, "fail_if_not_exists", 0);
        }
    }

    if (client_api_call(client, "POST", body, NULL, "/channels/%s/messages", channel_id)) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("sent message '%s' in %s.", content ? content : "<embed>", channel_id);

    return (0);
}

static uint8_t edit_message(Client *client, const char *channel_id, const char *message_id, const char *content,
    const Embed *embeds, size_t embed_count)
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

    if (client_api_call(client, "PATCH", body, NULL, "/channels/%s/messages/%s", channel_id, message_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API uint8_t client_api_send_message(Client *client, const char *channel_id, const char *content)
{
    return (send_message(client, channel_id, content, NULL, NULL, 0));
}

CN_API uint8_t client_api_reply_message(Client *client, const char *channel_id, const char *message_id, const char *content)
{
    return (send_message(client, channel_id, content, message_id, NULL, 0));
}

CN_API uint8_t client_api_edit_message(Client *client, const char *channel_id, const char *message_id, const char *content)
{
    return (edit_message(client, channel_id, message_id, content, NULL, 0));
}

CN_API uint8_t client_api_send_embeds(Client *client, const char *channel_id, const char *content, const Embed *embeds, size_t count)
{
    return (send_message(client, channel_id, content, NULL, embeds, count));
}

CN_API uint8_t client_api_reply_embeds(Client *client, const char *channel_id, const char *message_id, const char *content,
    const Embed *embeds, size_t count)
{
    return (send_message(client, channel_id, content, message_id, embeds, count));
}

CN_API uint8_t client_api_edit_message_embeds(Client *client, const char *channel_id, const char *message_id, const char *content,
    const Embed *embeds, size_t count)
{
    return (edit_message(client, channel_id, message_id, content, embeds, count));
}

CN_API uint8_t client_api_delete_message(Client *client, const char *channel_id, const char *message_id)
{
    if (client_api_call(client, "DELETE", NULL, NULL, "/channels/%s/messages/%s", channel_id, message_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

static uint8_t send_dm(Client *client, const char *user_id, const char *content, const Embed *embeds, size_t embed_count)
{
    Buffer resp = {.capacity = 0, .data = 0, .size = 0};
    cJSON *body = cJSON_CreateObject();
    cJSON *channel;
    const cJSON *id;
    char channel_id[32];
    uint8_t status;

    if (!body || !cJSON_AddStringToObject(body, "recipient_id", user_id)) {
        cJSON_Delete(body);
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate dm body.");
        return (1);
    }

    if (client_api_call(client, "POST", body, &resp, "/users/@me/channels")) {
        PROPAGATE_ERR();
        return (1);
    }

    channel = resp.data ? cJSON_Parse(resp.data) : NULL;
    empty_buffer(&resp);
    id = channel ? cJSON_GetObjectItemCaseSensitive(channel, "id") : NULL;

    if (!cJSON_IsString(id) || strlen(id->valuestring) >= sizeof channel_id) {
        cJSON_Delete(channel);
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed dm channel received.");
        return (1);
    }

    snprintf(channel_id, sizeof channel_id, "%s", id->valuestring);
    cJSON_Delete(channel);

    status = send_message(client, channel_id, content, NULL, embeds, embed_count);

    if (status) {
        PROPAGATE_ERR();
    }
    return (status);
}

CN_API uint8_t client_api_send_dm(Client *client, const char *user_id, const char *content)
{
    return (send_dm(client, user_id, content, NULL, 0));
}

CN_API uint8_t client_api_send_dm_embeds(Client *client, const char *user_id, const char *content, const Embed *embeds, size_t count)
{
    return (send_dm(client, user_id, content, embeds, count));
}

CN_API uint8_t client_api_pin_message(Client *client, const char *channel_id, const char *message_id)
{
    if (client_api_call(client, "PUT", NULL, NULL, "/channels/%s/messages/pins/%s", channel_id, message_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}

CN_API uint8_t client_api_unpin_message(Client *client, const char *channel_id, const char *message_id)
{
    if (client_api_call(client, "DELETE", NULL, NULL, "/channels/%s/messages/pins/%s", channel_id, message_id)) {
        PROPAGATE_ERR();
        return (1);
    }
    return (0);
}