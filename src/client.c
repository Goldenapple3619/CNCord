#include "client.h"
#include "error.h"
#include "clock.h"
#include "buffer.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

CN_API void client_cache_lock(Client *client)
{
    mutex_lock(&client->cache.lock);
}

CN_API void client_cache_unlock(Client *client)
{
    mutex_unlock(&client->cache.lock);
}

CN_API void client_commands_lock(Client *client)
{
    mutex_lock(&client->commands_lock);
}

CN_API void client_commands_unlock(Client *client)
{
    mutex_unlock(&client->commands_lock);
}

void apply_common_opts(CURL *h)
{
    curl_easy_setopt(h, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(h, CURLOPT_NOSIGNAL, 1L);

    const char *ca = getenv("DISCORD_CA_BUNDLE");

    if (ca && *ca)
        curl_easy_setopt(h, CURLOPT_CAINFO, ca);
    #if defined(_WIN32) && defined(CURLSSLOPT_NATIVE_CA)
        curl_easy_setopt(h, CURLOPT_SSL_OPTIONS, (long)CURLSSLOPT_NATIVE_CA);
    #endif
}

CN_API void empty_client(Client *client)
{
    if (!client) {
        RAISE(ERR_INVALID_POINTER, "can't empty null client");
        return;
    }

    client->intents = 0;

    if (client->token) {
        (void)free(client->token);
        client->token = NULL;
    }
    if (client->id) {
        (void)free(client->id);
        client->id = NULL;
    }
    if (client->username) {
        (void)free(client->username);
        client->username = NULL;
    }
    if (client->application_id) {
        (void)free(client->application_id);
        client->application_id = NULL;
    }

    if (client->connection.http) {
        (void)curl_easy_cleanup(client->connection.http);
        client->connection.http = NULL;
    }
    if (client->connection.ws) {
        (void)curl_easy_cleanup(client->connection.ws);
        client->connection.ws = NULL;
    }
    if (client->connection.http_headers) {
        (void)curl_slist_free_all(client->connection.http_headers);
        client->connection.http_headers = NULL;
    }

    memset(&client->events, 0, sizeof(client->events));

    empty_generic_vector(&client->cache.guilds, (void (*)(void *))&guild_dedup);
    empty_generic_vector(&client->cache.dm_channels, (void (*)(void *))&channel_dedup);
    empty_generic_vector(&client->commands, free);

    mutex_destroy(&client->cache.lock);
    mutex_destroy(&client->commands_lock);

    (void)client_clear_session(client);
}

CN_API void delete_client(Client *client)
{
    if (!client) {
        RAISE(ERR_INVALID_POINTER, "can't free empty client");
        return;
    }

    (void)empty_client(client);
    (void)free(client);
}

CN_API void client_clear_session(Client *client)
{
    if (!client) {
        RAISE(ERR_INVALID_POINTER, "can't clear empty client session");
        return;
    }

    if (client->session.id) {
        (void)free(client->session.id);
        client->session.id = NULL;
    }
    if (client->session.resume_url) {
        (void)free(client->session.resume_url);
        client->session.resume_url = NULL;
    }
    client->session.seq = -1;
    client->session.close_code = 0;
    client->session.heartbeat_ms = 0;
    client->session.next_heartbeat = 0;
    client->session.ack_pending = false;
}

CN_API uint8_t client_build_http_headers(Client *client)
{
    if (!client) {
        RAISE(ERR_INVALID_POINTER, "can't build empty client http header");
        return (1);
    }

    char auth[256];

    if (client->connection.http_headers) {
        (void)curl_slist_free_all(client->connection.http_headers);
    }

    (void)snprintf(auth, sizeof(auth), "Authorization: Bot %s", client->token);
    client->connection.http_headers = curl_slist_append(NULL, auth);
    client->connection.http_headers = curl_slist_append(client->connection.http_headers, "Content-Type: application/json");
    return (0);
}

CN_API uint8_t client_set_session(Client *client,
    const char *id, const char *username, const char *resume_url, const char *session_id)
{
    (void)username;

    if (!client) {
        RAISE(ERR_INVALID_POINTER, "can't set empty client session");
        return (1);
    }

    if (client->id)
        (void)free(client->id);
    client->id = id ? strdup(id) : NULL;
    if (id && !client->id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new client id.");
        return (1);
    }
    if (client->username)
        (void)free(client->username);
    client->username = username ? strdup(username) : NULL;
    if (username && !client->username) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new client username.");
        return (1);
    }

    if (client->session.id)
        (void)free(client->session.id);
    client->session.id = session_id ? strdup(session_id) : NULL;
    if (session_id && !client->session.id) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new client session_id.");
        return (1);
    }

    if (client->session.resume_url)
        (void)free(client->session.resume_url);
    client->session.resume_url = resume_url ? strdup(resume_url) : NULL;
    if (resume_url && !client->session.resume_url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new client resume_url.");
        return (1);
    }

    return (0);
}

CN_API Client *new_client(const char *token, uint64_t intents)
{
    if (!token || !(*token)) {
        RAISE(ERR_INVALID_POINTER, "can't create a client with no token.");
        return (NULL);
    }

    Client *client = (Client *)malloc(sizeof(Client));

    if (!client) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate client.")
        return (NULL);
    }

    client->intents = intents;
    flag_set(&client->running, false);

    client->session.seq = -1;
    client->session.close_code = 0;
    client->session.id = NULL;
    client->session.resume_url = NULL;
    client->session.heartbeat_ms = 0;
    client->session.next_heartbeat = 0;
    client->session.ack_pending = false;

    memset(&client->cache, 0, sizeof(client->cache));
    memset(&client->commands, 0, sizeof(client->commands));

    mutex_init(&client->cache.lock);
    mutex_init(&client->commands_lock);

    memset(&client->events, 0, sizeof(client->events));

    client->token = strdup(token);
    client->id = NULL;
    client->application_id = NULL;
    client->username = NULL;

    if (!client->token) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate client token.")
        mutex_destroy(&client->cache.lock);
        mutex_destroy(&client->commands_lock);
        (void)free(client);
        return (NULL);
    }

    client->connection.http  = curl_easy_init();
    client->connection.http_headers = NULL;
    client->connection.ws = NULL;

    if (!client->connection.http) {
        RAISE(ERR_OS, "failed to create http conn.")
        mutex_destroy(&client->cache.lock);
        mutex_destroy(&client->commands_lock);
        (void)free(client->token);
        (void)free(client);
        return (NULL);
    }

    (void)client_build_http_headers(client);

    return (client);
}

CN_API void client_wait_socket(Client *client, short events, int timeout_ms)
{
    if (!client) {
        RAISE(ERR_INVALID_POINTER, "can't wait socket on empty client");
        return;
    }

    curl_socket_t fd = CURL_SOCKET_BAD;
    PollFd p;

    if (timeout_ms < 0)
        timeout_ms = 0;

    if (curl_easy_getinfo(client->connection.ws, CURLINFO_ACTIVESOCKET, &fd) != CURLE_OK || fd == CURL_SOCKET_BAD) {
        sleep_ms(timeout_ms < 50 ? timeout_ms : 50);
        return;
    }

    memset(&p, 0, sizeof p);
    p.fd = fd;
    p.events = events;
    (void)POLL_FN(&p, 1, timeout_ms);
}

CN_API uint8_t client_socket_send_text(Client *client, const char *s)
{
    size_t len = strlen(s);
    size_t off = 0;
    size_t sent;
    CURLcode rc;

    while (off < len) {
        sent = 0;
        rc = curl_ws_send(client->connection.ws, s + off, len - off, &sent, 0, CURLWS_TEXT);

        off += sent;

        if (rc == CURLE_AGAIN) {
            (void)client_wait_socket(client, EV_OUT, 100);
            continue;
        }

        if (rc != CURLE_OK) {
            RAISE_FMT(ERR_OS, "failed to send %s.", curl_easy_strerror(rc));
            return (1);
        }
    }
    return (0);
}

CN_API uint8_t client_socket_recv_text(Client *client, Buffer *msg)
{
    char chunk[WS_READ_CHUNK_SIZE];
    const struct curl_ws_frame *meta;
    size_t n = 0;
    CURLcode rc;

    memset(chunk, 0, sizeof(chunk));

    for (;;) {
        n = 0;
        meta = NULL;
        rc = curl_ws_recv(client->connection.ws, chunk, sizeof chunk, &n, &meta);

        if (rc == CURLE_AGAIN)
            return (2);

        if (rc != CURLE_OK) {
            RAISE_FMT(ERR_OS, "failed to recv %s.", curl_easy_strerror(rc))
            return (1);
        }

        if (meta->flags & CURLWS_CLOSE) {
            client->session.close_code = (n >= 2)
                ? (((unsigned char)chunk[0] << 8) | (unsigned char)chunk[1]) : 0;
            switch (client->session.close_code) {
                case 4004: case 4010: case 4011: case 4012: case 4013: case 4014:
                    RAISE_FMT(ERR_NOT_COMPATIBLE, "server returned status code saying that the identity is invalid %d.", client->session.close_code)
                    return (3);
                default:
                    RAISE_FMT(ERR_NOT_COMPATIBLE, "server returned status code %d.", client->session.close_code)
                    return (1);
            }
        }

        if (meta->flags & (CURLWS_PING | CURLWS_PONG))
            continue;

        if (msg->size + n > MAX_GATEWAY_MSG) {
            RAISE_FMT(ERR_OUT_OF_BOUND, "message too large (%zu > %zu).", (size_t)(msg->size + n), (size_t)MAX_GATEWAY_MSG);
            return (1);
        }

        if (n && add_buffer(msg, chunk, n)) {
            PROPAGATE_ERR();
            return (1);
        }

        if (!(meta->flags & CURLWS_CONT) && meta->bytesleft == 0)
            return (0);
    }
}

CN_API void client_socket_close(Client *client)
{
    if (client->connection.ws) {
        curl_easy_cleanup(client->connection.ws);
        client->connection.ws = NULL;
    }
}

CN_API uint8_t client_socket_setup(Client *client)
{
    char url[512];
    CURLcode rc;

    if (client->session.id && client->session.resume_url)
        snprintf(url, sizeof(url), "%s/?v=10&encoding=json", client->session.resume_url);
    else
        snprintf(url, sizeof(url), "%s", GATEWAY_URL);

    if (client->connection.ws) {
        curl_easy_cleanup(client->connection.ws);
        client->connection.ws = NULL;
    }

    client->connection.ws = curl_easy_init();
    if (!client->connection.ws) {
        RAISE(ERR_OS, "failed to create ws connection.");
        return (1);
    }

    apply_common_opts(client->connection.ws);
    curl_easy_setopt(client->connection.ws, CURLOPT_URL, url);
    curl_easy_setopt(client->connection.ws, CURLOPT_CONNECT_ONLY, 2L);

    LOG_I("connecting to '%s'", url);

    rc = curl_easy_perform(client->connection.ws);
    if (rc != CURLE_OK) {
        RAISE_FMT(ERR_OS, "failed to connect ws %s.", curl_easy_strerror(rc));
        curl_easy_cleanup(client->connection.ws);
        client->connection.ws = NULL;
        return (1);
    }

    client->session.heartbeat_ms = 0;
    client->session.ack_pending = false;
    client->session.close_code = 0;

    return (0);
}

static uint8_t session_heartbeat(Client *client, uint64_t *wait_ms)
{
    int64_t until = 0;
    uint64_t now = now_ms();

    if (client->session.heartbeat_ms > 0) {
        if (now >= client->session.next_heartbeat) {
            if (client->session.ack_pending) {
                LOG_E("no heartbeat ACK, reconnecting");
                return (1);
            }
            if (client_gw_send_heartbeat(client)) {
                PROPAGATE_ERR();
                return (1);
            }
            client->session.ack_pending = true;
            client->session.next_heartbeat = now + client->session.heartbeat_ms;
        }
        until = (int64_t)client->session.next_heartbeat - (int64_t)now_ms();
        if (until < (int64_t)*wait_ms)
            *wait_ms = (uint64_t)(until < 0 ? 0 : until);
    }

    return (0);
}

CN_API SessionResult run_session(Client *client)
{
    Buffer msg = {.capacity = 0, .size = 0, .data = NULL};
    uint8_t recv_status;
    Action temp_action;
    uint64_t wait_ms;
    size_t sent;

    if (client_socket_setup(client)) {
        PROPAGATE_ERR();
        return (SESSION_RECONNECT);
    }

    while (flag_get(&client->running)) {
        wait_ms = 1000;

        while ((recv_status = client_socket_recv_text(client, &msg)) == 0) {
            temp_action = client_gw_handle(client, msg.data);

            empty_buffer(&msg);

            if (temp_action == ACT_NEW_SESSION)
                client_clear_session(client);

            if (temp_action != ACT_NONE) {
                sleep_ms(1000 + rand() % 4000);

                empty_buffer(&msg);
                client_socket_close(client);

                return (SESSION_RECONNECT);
            }
        }

        if (recv_status == 3) {
            PROPAGATE_ERR();

            empty_buffer(&msg);
            client_socket_close(client);

            return (SESSION_FATAL);
        }
        if (recv_status == 1) {
            PROPAGATE_ERR();

            if (client->session.close_code == 4007 || client->session.close_code == 4009)
                client_clear_session(client);

            empty_buffer(&msg);
            client_socket_close(client);

            return (SESSION_RECONNECT);
        }

        if (session_heartbeat(client, &wait_ms)) {
            PROPAGATE_ERR();

            empty_buffer(&msg);
            client_socket_close(client);

            return (SESSION_RECONNECT);
        }

        while (has_error()) {
            on_error(client, get_error());
        }

        client_wait_socket(client, EV_IN, wait_ms);
    }

    sent = 0;
    curl_ws_send(client->connection.ws, "", 0, &sent, 0, CURLWS_CLOSE);

    empty_buffer(&msg);
    client_socket_close(client);

    return (SESSION_QUIT);
}


CN_API uint8_t start_client(Client *client)
{
    SessionResult r;

    while (flag_get(&client->running)) {
        r = run_session(client);

        if (r == SESSION_QUIT)
            return (0);

        if (r == SESSION_FATAL) {
            PROPAGATE_ERR();
            return (1);
        }

        LOG_I("reconnecting session...");

        sleep_ms(1000 + rand() % 4000);
    }

    return (0);
}