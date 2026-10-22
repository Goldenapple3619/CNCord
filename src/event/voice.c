#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

uint8_t on_voice_state_update(Client *client, const cJSON *d)
{
    VoiceState state = {0};

    state.user_id = json_str(d, "user_id");
    state.guild_id = json_str(d, "guild_id");
    state.channel_id = json_str(d, "channel_id");
    state.session_id = json_str(d, "session_id");
    state.deaf = json_flag(d, "deaf");
    state.mute = json_flag(d, "mute");
    state.self_deaf = json_flag(d, "self_deaf");
    state.self_mute = json_flag(d, "self_mute");
    state.self_stream = json_flag(d, "self_stream");
    state.self_video = json_flag(d, "self_video");
    state.suppress = json_flag(d, "suppress");

    if (!state.user_id) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    LOG_D("<VOICE user_id='%s' channel_id='%s'>", state.user_id, state.channel_id ? state.channel_id : "null");

    FIRE(client, on_voice_state_update, &state);
    return (0);
}

uint8_t on_voice_server_update(Client *client, const cJSON *d)
{
    if (!json_str(d, "guild_id") || !json_str(d, "token")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    FIRE(client, on_voice_server_update, json_str(d, "guild_id"), json_str(d, "token"), json_str(d, "endpoint"));
    return (0);
}
