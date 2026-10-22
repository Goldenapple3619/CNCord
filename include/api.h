#ifndef _API_H_
    #define _API_H_

    #include "core.h"
    #include "client.h"
    #include "buffer.h"

    #define MSG_FLAG_EPHEMERAL 64
    #define EMBED_URL_MAX 2048
    #define EMBED_TITLE_MAX 256
    #define EMBED_DESCRIPTION_MAX 4096
    #define EMBED_FIELD_NAME_MAX 256
    #define EMBED_FIELD_VALUE_MAX 1024
    #define EMBED_FOOTER_MAX 2048
    #define EMBED_AUTHOR_MAX 256
    #define ZERO_WIDTH_SPACE "\xE2\x80\x8B"

    cJSON *new_message_body(const char *content);

    CN_API long client_api_request(Client *client, const char *method, const char *path, const char *body);
    CN_API uint8_t client_api_call(Client *client, const char *method, cJSON *body, Buffer *out, const char *fmt, ...);

    CN_API uint8_t client_api_send_message(Client *client, const char *channel_id, const char *content);
    CN_API uint8_t client_api_reply_message(Client *client, const char *channel_id, const char *message_id, const char *content);
    CN_API uint8_t client_api_edit_message(Client *client, const char *channel_id, const char *message_id, const char *content);
    CN_API uint8_t client_api_delete_message(Client *client, const char *channel_id, const char *message_id);

    CN_API uint8_t client_api_add_reaction(Client *client, const char *channel_id, const char *message_id, const char *emoji);

    CN_API uint8_t client_api_trigger_typing(Client *client, const char *channel_id);

    CN_API uint8_t client_api_send_dm(Client *client, const char *user_id, const char *content);
    CN_API uint8_t client_api_send_dm_embeds(Client *client, const char *user_id, const char *content, const Embed *embeds, size_t count);

    CN_API uint8_t client_api_pin_message(Client *client, const char *channel_id, const char *message_id);
    CN_API uint8_t client_api_unpin_message(Client *client, const char *channel_id, const char *message_id);

    CN_API uint8_t client_api_send_embeds(Client *client, const char *channel_id, const char *content, const Embed *embeds, size_t count);
    CN_API uint8_t client_api_reply_embeds(Client *client, const char *channel_id, const char *message_id, const char *content, const Embed *embeds, size_t count);
    CN_API uint8_t client_api_edit_message_embeds(Client *client, const char *channel_id, const char *message_id, const char *content, const Embed *embeds, size_t count);

    CN_API uint8_t client_api_kick_member(Client *client, const char *guild_id, const char *user_id);
    CN_API uint8_t client_api_ban_member(Client *client, const char *guild_id, const char *user_id, uint32_t delete_message_seconds);
    CN_API uint8_t client_api_unban_member(Client *client, const char *guild_id, const char *user_id);
    CN_API uint8_t client_api_add_role(Client *client, const char *guild_id, const char *user_id, const char *role_id);
    CN_API uint8_t client_api_remove_role(Client *client, const char *guild_id, const char *user_id, const char *role_id);

    CN_API uint8_t client_interaction_reply(Client *client, const Interaction *interaction, const char *content, cnbool ephemeral);
    CN_API uint8_t client_interaction_defer(Client *client, const Interaction *interaction, cnbool ephemeral);
    CN_API uint8_t client_interaction_edit_reply(Client *client, const Interaction *interaction, const char *content);
    CN_API uint8_t client_interaction_followup(Client *client, const Interaction *interaction, const char *content, cnbool ephemeral);
    CN_API uint8_t client_interaction_edit_reply_embeds(Client *client, const Interaction *interaction, const char *content, const Embed *embeds, size_t count);
    CN_API uint8_t client_interaction_reply_embeds(Client *client, const Interaction *interaction, const char *content, const Embed *embeds, size_t count, cnbool ephemeral);
    CN_API uint8_t client_interaction_followup_embeds(Client *client, const Interaction *interaction, const char *content, const Embed *embeds, size_t count, cnbool ephemeral);

    CN_API uint8_t client_api_leave_guild(Client *client, const char *guild_id);

    CN_API uint8_t client_api_delete_channel(Client *client, const char *channel_id);

    uint8_t message_body_add_embeds(cJSON *body, const Embed *embeds, size_t count);
#endif
