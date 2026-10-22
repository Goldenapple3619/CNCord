#ifndef _EVENT_H_
    #define _EVENT_H_

    #include "client.h"

    struct reg_event_s {
        const char *name;
        uint8_t (*handler)(Client *client, const cJSON *d);
    };

    #define FIRE(client, ev, ...)                                                   \
        do {                                                                        \
            if ((client)->events.ev && (client)->events.ev((client), __VA_ARGS__)) {\
                PROPAGATE_ERR();                                                    \
                return (1);                                                         \
            }                                                                       \
        } while (0)

    uint8_t on_webhooks_update(Client *client, const cJSON *d);

    uint8_t on_channel_pins_update(Client *client, const cJSON *d);

    uint8_t on_thread_list_sync(Client *client, const cJSON *d);
    uint8_t on_thread_delete(Client *client, const cJSON *d);
    uint8_t on_thread_update(Client *client, const cJSON *d);
    uint8_t on_thread_create(Client *client, const cJSON *d);

    uint8_t on_channel_delete(Client *client, const cJSON *d);
    uint8_t on_channel_update(Client *client, const cJSON *d);
    uint8_t on_channel_create(Client *client, const cJSON *d);

    uint8_t on_error(Client *client, const ErrorContext *ctx);
    uint8_t on_resume(Client *client, const cJSON *d);
    uint8_t on_ready(Client *client, const cJSON *d);

    uint8_t on_guild_create(Client *client, const cJSON *d);
    uint8_t on_guild_update(Client *client, const cJSON *d);
    uint8_t on_guild_delete(Client *client, const cJSON *d);

    uint8_t on_emojis_update(Client *client, const cJSON *d);

    uint8_t on_integrations_update(Client *client, const cJSON *d);

    uint8_t on_interaction_create(Client *client, const cJSON *d);

    uint8_t on_invite_create(Client *client, const cJSON *d);
    uint8_t on_invite_delete(Client *client, const cJSON *d);

    uint8_t on_member_add(Client *client, const cJSON *d);
    uint8_t on_member_update(Client *client, const cJSON *d);
    uint8_t on_member_remove(Client *client, const cJSON *d);

    uint8_t on_ban_add(Client *client, const cJSON *d);
    uint8_t on_ban_remove(Client *client, const cJSON *d);

    uint8_t on_message_create(Client *client, const cJSON *d);
    uint8_t on_message_update(Client *client, const cJSON *d);
    uint8_t on_message_delete(Client *client, const cJSON *d);
    uint8_t on_message_delete_bulk(Client *client, const cJSON *d);

    uint8_t on_reaction_add(Client *client, const cJSON *d);
    uint8_t on_reaction_remove(Client *client, const cJSON *d);
    uint8_t on_reaction_remove_all(Client *client, const cJSON *d);
    uint8_t on_reaction_remove_emoji(Client *client, const cJSON *d);

    uint8_t on_role_create(Client *client, const cJSON *d);
    uint8_t on_role_update(Client *client, const cJSON *d);
    uint8_t on_role_delete(Client *client, const cJSON *d);

    uint8_t on_scheduled_event_create(Client *client, const cJSON *d);
    uint8_t on_scheduled_event_update(Client *client, const cJSON *d);
    uint8_t on_scheduled_event_delete(Client *client, const cJSON *d);
    uint8_t on_scheduled_event_user_add(Client *client, const cJSON *d);
    uint8_t on_scheduled_event_user_remove(Client *client, const cJSON *d);

    uint8_t on_stage_create(Client *client, const cJSON *d);
    uint8_t on_stage_update(Client *client, const cJSON *d);
    uint8_t on_stage_delete(Client *client, const cJSON *d);

    uint8_t on_typing_start(Client *client, const cJSON *d);
    uint8_t on_presence_update(Client *client, const cJSON *d);
    uint8_t on_user_update(Client *client, const cJSON *d);

    uint8_t on_voice_state_update(Client *client, const cJSON *d);
    uint8_t on_voice_server_update(Client *client, const cJSON *d);
#endif