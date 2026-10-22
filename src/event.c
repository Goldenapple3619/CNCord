#include "client.h"
#include "error.h"
#include "event.h"
#include "log.h"

static const struct reg_event_s EVENTS[] = {
    {"READY", &on_ready},
    {"RESUMED", &on_resume},

    {"MESSAGE_CREATE", &on_message_create},
    {"MESSAGE_UPDATE", &on_message_update},
    {"MESSAGE_DELETE", &on_message_delete},
    {"MESSAGE_DELETE_BULK", &on_message_delete_bulk},
    {"MESSAGE_REACTION_ADD", &on_reaction_add},
    {"MESSAGE_REACTION_REMOVE", &on_reaction_remove},
    {"MESSAGE_REACTION_REMOVE_ALL", &on_reaction_remove_all},
    {"MESSAGE_REACTION_REMOVE_EMOJI", &on_reaction_remove_emoji},
    {"TYPING_START", &on_typing_start},

    {"GUILD_CREATE", &on_guild_create},
    {"GUILD_UPDATE", &on_guild_update},
    {"GUILD_DELETE", &on_guild_delete},
    {"GUILD_EMOJIS_UPDATE", &on_emojis_update},
    {"GUILD_INTEGRATIONS_UPDATE", &on_integrations_update},
    {"GUILD_BAN_ADD", &on_ban_add},
    {"GUILD_BAN_REMOVE", &on_ban_remove},

    {"CHANNEL_CREATE", &on_channel_create},
    {"CHANNEL_UPDATE", &on_channel_update},
    {"CHANNEL_DELETE", &on_channel_delete},
    {"CHANNEL_PINS_UPDATE", &on_channel_pins_update},
    {"THREAD_CREATE", &on_thread_create},
    {"THREAD_UPDATE", &on_thread_update},
    {"THREAD_DELETE", &on_thread_delete},
    {"THREAD_LIST_SYNC", &on_thread_list_sync},
    {"WEBHOOKS_UPDATE", &on_webhooks_update},

    {"GUILD_MEMBER_ADD", &on_member_add},
    {"GUILD_MEMBER_UPDATE", &on_member_update},
    {"GUILD_MEMBER_REMOVE", &on_member_remove},
    {"GUILD_ROLE_CREATE", &on_role_create},
    {"GUILD_ROLE_UPDATE", &on_role_update},
    {"GUILD_ROLE_DELETE", &on_role_delete},
    {"USER_UPDATE", &on_user_update},
    {"PRESENCE_UPDATE", &on_presence_update},

    {"INVITE_CREATE", &on_invite_create},
    {"INVITE_DELETE", &on_invite_delete},

    {"INTERACTION_CREATE", &on_interaction_create},

    {"VOICE_STATE_UPDATE", &on_voice_state_update},
    {"VOICE_SERVER_UPDATE", &on_voice_server_update},
    {"STAGE_INSTANCE_CREATE", &on_stage_create},
    {"STAGE_INSTANCE_UPDATE", &on_stage_update},
    {"STAGE_INSTANCE_DELETE", &on_stage_delete},
    {"GUILD_SCHEDULED_EVENT_CREATE", &on_scheduled_event_create},
    {"GUILD_SCHEDULED_EVENT_UPDATE", &on_scheduled_event_update},
    {"GUILD_SCHEDULED_EVENT_DELETE", &on_scheduled_event_delete},
    {"GUILD_SCHEDULED_EVENT_USER_ADD", &on_scheduled_event_user_add},
    {"GUILD_SCHEDULED_EVENT_USER_REMOVE", &on_scheduled_event_user_remove}
};

CN_API uint8_t handle_dispatch(Client *client, const char *event, const cJSON *d)
{
    uint8_t status;

    for (size_t i = 0; i < sizeof(EVENTS) / sizeof(EVENTS[0]); ++i) {
        if (strcmp(event, EVENTS[i].name))
            continue;

        status = EVENTS[i].handler(client, d);

        if (status) {
            PROPAGATE_ERR();
        }

        return (status);
    }

    LOG_W("<%s UNHANDLED>", event);
    return (0);
}
