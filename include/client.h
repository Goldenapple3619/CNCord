#ifndef _CLIENT_H_
    #define _CLIENT_H_

    #ifdef _WIN32
        #ifndef _WIN32_WINNT
            #define _WIN32_WINNT 0x0601
        #endif

        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN
        #endif

        #ifdef _MSC_VER
            #define _CRT_SECURE_NO_WARNINGS
            #pragma comment(lib, "ws2_32.lib")
        #endif

        #include <winsock2.h>
        #include <windows.h>

    #else
        #ifndef _POSIX_C_SOURCE
            #define _POSIX_C_SOURCE 200809L
        #endif
        #include <poll.h>
    #endif

    #ifdef _WIN32
        typedef WSAPOLLFD PollFd;
        #define POLL_FN(p, n, t)  WSAPoll((p), (ULONG)(n), (t))
    #else
        typedef struct pollfd PollFd;
        #define POLL_FN(p, n, t)  poll((p), (nfds_t)(n), (t))
    #endif

    #define EV_IN ((short)POLLIN)
    #define EV_OUT ((short)POLLOUT)
    #define WS_READ_CHUNK_SIZE 4096

    #define GATEWAY_URL   "wss://gateway.discord.gg/?v=10&encoding=json"
    #define API_BASE      "https://discord.com/api/v10"
    #define USER_AGENT    "DiscordBot"
    #define DEFAULT_INTENTS   (CI_GUILDS | CI_GUILD_MESSAGES | CI_MESSAGE_CONTENT)
    #define MAX_GATEWAY_MSG  ((size_t)32 * 1024 * 1024)

    typedef struct cJSON cJSON;
    typedef void CURL;

    #include "core.h"
    #include "error.h"
    #include "thread.h"
    #include "types.h"
    #include "generic_vector.h"

    typedef enum {
        SESSION_RECONNECT,
        SESSION_FATAL,
        SESSION_QUIT
    } SessionResult;

    typedef enum {
        ACT_NONE,
        ACT_RECONNECT,
        ACT_NEW_SESSION
    } Action;

    typedef enum {
        CI_GUILDS = (1 << 0),
        CI_GUILD_MEMBERS = (1 << 1),
        CI_GUILD_MODERATION = (1 << 2),
        CI_GUILD_EXPRESSIONS = (1 << 3),
        CI_GUILD_INTEGRATIONS = (1 << 4),
        CI_GUILD_WEBHOOKS = (1 << 5),
        CI_GUILD_INVITES = (1 << 6),
        CI_GUILD_VOICE_STATES = (1 << 7),
        CI_GUILD_PRESENCES = (1 << 8),
        CI_GUILD_MESSAGES = (1 << 9),
        CI_GUILD_MESSAGE_REACTIONS = (1 << 10),
        CI_GUILD_MESSAGE_TYPING = (1 << 11),
        CI_DIRECT_MESSAGES = (1 << 12),
        CI_DIRECT_MESSAGE_REACTIONS = (1 << 13),
        CI_DIRECT_MESSAGE_TYPING = (1 << 14),
        CI_MESSAGE_CONTENT = (1 << 15),
        CI_GUILD_SCHEDULED_EVENTS = (1 << 16),
        CI_AUTO_MODERATION_CONFIGURATION = (1 << 20),
        CI_AUTO_MODERATION_EXECUTION = (1 << 21),
        CI_GUILD_MESSAGE_POLLS = (1 << 24),
        CI_DIRECT_MESSAGE_POLLS = (1 << 25)
    } ClientIntents;

    struct Client;

    typedef struct Command {
        const char *name;
        const char *description;
        const CommandOption *options;
        size_t option_count;
        uint8_t (*handler)(struct Client *client, const Interaction *interaction);
    } Command;

    typedef struct Client {
        char *token;
        char *id;
        char *application_id;
        char *username;
        uint64_t intents;

        struct {
            void *ws;
            void *http;
            struct curl_slist *http_headers;
        } connection;

        struct {
            char *id;
            char *resume_url;

            int64_t seq; /* last sequence number, -1 = none */
            uint64_t heartbeat_ms;
            uint64_t next_heartbeat;

            cnbool ack_pending;

            int close_code;
        } session;

        struct {
            uint8_t (*on_error)(struct Client *client, const ErrorContext *ctx);
            uint8_t (*on_ready)(struct Client *client);
            uint8_t (*on_resume)(struct Client *client);

            uint8_t (*on_message_create)(struct Client *client, const Message *msg);
            uint8_t (*on_message_update)(struct Client *client, const Message *msg);
            uint8_t (*on_message_delete)(struct Client *client, const Message *msg);
            uint8_t (*on_reaction_add)(struct Client *client, const Reaction *reaction);
            uint8_t (*on_reaction_remove)(struct Client *client, const Reaction *reaction);
            uint8_t (*on_reaction_remove_all)(struct Client *client, const Reaction *reaction);
            uint8_t (*on_reaction_remove_emoji)(struct Client *client, const Reaction *reaction);
            uint8_t (*on_typing_start)(struct Client *client, const char *guild_id, const char *channel_id, const char *user_id);

            uint8_t (*on_guild_create)(struct Client *client, const Guild *guild);
            uint8_t (*on_guild_update)(struct Client *client, const Guild *guild);
            uint8_t (*on_guild_delete)(struct Client *client, const Guild *guild);
            uint8_t (*on_emojis_update)(struct Client *client, const Guild *guild);
            uint8_t (*on_integrations_update)(struct Client *client, const char *guild_id);
            uint8_t (*on_ban_add)(struct Client *client, const char *guild_id, const User *user);
            uint8_t (*on_ban_remove)(struct Client *client, const char *guild_id, const User *user);

            uint8_t (*on_channel_create)(struct Client *client, const Channel *channel);
            uint8_t (*on_channel_update)(struct Client *client, const Channel *channel);
            uint8_t (*on_channel_delete)(struct Client *client, const Channel *channel);
            uint8_t (*on_channel_pins_update)(struct Client *client, const Channel *channel);
            uint8_t (*on_thread_create)(struct Client *client, const Channel *thread);
            uint8_t (*on_thread_update)(struct Client *client, const Channel *thread);
            uint8_t (*on_thread_delete)(struct Client *client, const Channel *thread);
            uint8_t (*on_webhooks_update)(struct Client *client, const char *guild_id, const char *channel_id);

            uint8_t (*on_member_add)(struct Client *client, const char *guild_id, const Member *member);
            uint8_t (*on_member_update)(struct Client *client, const char *guild_id, const Member *member);
            uint8_t (*on_member_remove)(struct Client *client, const char *guild_id, const Member *member);
            uint8_t (*on_role_create)(struct Client *client, const char *guild_id, const Role *role);
            uint8_t (*on_role_update)(struct Client *client, const char *guild_id, const Role *role);
            uint8_t (*on_role_delete)(struct Client *client, const char *guild_id, const Role *role);
            uint8_t (*on_user_update)(struct Client *client, const User *user);
            uint8_t (*on_presence_update)(struct Client *client, const char *guild_id, const char *user_id, const char *status);

            uint8_t (*on_invite_create)(struct Client *client, const Invite *invite);
            uint8_t (*on_invite_delete)(struct Client *client, const Invite *invite);

            uint8_t (*on_interaction_create)(struct Client *client, const Interaction *interaction);

            uint8_t (*on_voice_state_update)(struct Client *client, const VoiceState *state);
            uint8_t (*on_voice_server_update)(struct Client *client, const char *guild_id, const char *token, const char *endpoint);
            uint8_t (*on_stage_create)(struct Client *client, const StageInstance *stage);
            uint8_t (*on_stage_update)(struct Client *client, const StageInstance *stage);
            uint8_t (*on_stage_delete)(struct Client *client, const StageInstance *stage);
            uint8_t (*on_scheduled_event_create)(struct Client *client, const ScheduledEvent *event);
            uint8_t (*on_scheduled_event_update)(struct Client *client, const ScheduledEvent *event);
            uint8_t (*on_scheduled_event_delete)(struct Client *client, const ScheduledEvent *event);
            uint8_t (*on_scheduled_event_user_add)(struct Client *client, const char *guild_id, const char *event_id, const char *user_id);
            uint8_t (*on_scheduled_event_user_remove)(struct Client *client, const char *guild_id, const char *event_id, const char *user_id);
        } events;

        struct {
            struct generic_vector_s guilds;
            struct generic_vector_s dm_channels;

            Mutex lock;
        } cache;

        struct generic_vector_s commands;
        Mutex commands_lock;

        AtomicFlag running;
    } Client;

    uint8_t on_error(Client *client, const ErrorContext *ctx);

    CN_API uint8_t client_gw_send(Client *client, cJSON *root);
    CN_API uint8_t client_gw_send_heartbeat(Client *client);
    CN_API uint8_t client_gw_send_identify(Client *client);
    CN_API uint8_t client_gw_send_resume(Client *client);
    CN_API Action client_gw_handle(Client *client, const char *json);

    CN_API SessionResult run_session(Client *client);
    CN_API Client *new_client(const char *token, uint64_t intents);
    CN_API void client_clear_session(Client *client);
    CN_API void delete_client(Client *client);
    CN_API uint8_t client_set_session(Client *client,
        const char *id, const char *username, const char *resume_url, const char *session_id);
    CN_API uint8_t client_socket_send_text(Client *client, const char *s);
    CN_API uint8_t handle_dispatch(Client *client, const char *event, const cJSON *d);
    CN_API uint8_t start_client(Client *client);

    CN_API uint8_t client_register_command(Client *client, const Command *command);
    CN_API uint8_t client_sync_commands(Client *client, const char *guild_id);
    uint8_t client_dispatch_command(Client *client, const Interaction *interaction);

    void apply_common_opts(CURL *h);

    CN_API void client_cache_lock(Client *client);
    CN_API void client_cache_unlock(Client *client);

    CN_API void client_commands_lock(Client *client);
    CN_API void client_commands_unlock(Client *client);
#endif