#include "buffer.h"
#include "api.h"
#include "clock.h"
#include "log.h"

#include "curl/curl.h"
#include <cJSON.h>

static size_t http_write_cb(char *ptr, size_t size, size_t nmemb, void *ud)
{
    if (nmemb && size > SIZE_MAX / nmemb) {
        return (0);
    }

    size_t n = size * nmemb;

    return add_buffer((Buffer *)ud, ptr, n) ? 0 : n;
}

static long api_request(Client *client, const char *method, const char *path, const char *body, Buffer *out)
{
    char url[512];
    Buffer resp = {.capacity = 0, .data = 0, .size = 0};
    CURLcode rc;
    long code;
    double wait_s;
    cJSON *j;
    const cJSON *ra;
    int n = snprintf(url, sizeof url, API_BASE "%s", path);

    if (n < 0 || (size_t)n >= sizeof(url)) {
        RAISE(ERR_OUT_OF_BOUND, "api url too long.");
        return (-1);
    }

    for (int attempt = 0; attempt < 2; attempt++) {
        code = 0;

        curl_easy_reset(client->connection.http);
        apply_common_opts(client->connection.http);
        curl_easy_setopt(client->connection.http, CURLOPT_URL, url);
        curl_easy_setopt(client->connection.http, CURLOPT_HTTPHEADER, client->connection.http_headers);
        if (strcmp(method, "GET")) {
            curl_easy_setopt(client->connection.http, CURLOPT_CUSTOMREQUEST, method);
            curl_easy_setopt(client->connection.http, CURLOPT_POSTFIELDS, body ? body : "");
        }
        curl_easy_setopt(client->connection.http, CURLOPT_WRITEFUNCTION, http_write_cb);
        curl_easy_setopt(client->connection.http, CURLOPT_WRITEDATA, &resp);
        curl_easy_setopt(client->connection.http, CURLOPT_TIMEOUT, 15L);

        rc = curl_easy_perform(client->connection.http);

        if (rc != CURLE_OK) {
            RAISE_FMT(ERR_OS, "failed to send api request, %s", curl_easy_strerror(rc));

            empty_buffer(&resp);
            return (-1);
        }

        curl_easy_getinfo(client->connection.http, CURLINFO_RESPONSE_CODE, &code);

        if (code == 429 && attempt == 0) {
            wait_s = 1.0;
            j = resp.data ? cJSON_Parse(resp.data) : NULL;
            ra = j ? cJSON_GetObjectItemCaseSensitive(j, "retry_after") : NULL;

            if (cJSON_IsNumber(ra))
                wait_s = ra->valuedouble;

            cJSON_Delete(j);

            if (wait_s > 5.0)
                wait_s = 5.0;

            LOG_W("rate limited, waiting %.2fs\n", wait_s);

            sleep_ms((long)(wait_s * 1000));

            empty_buffer(&resp);
            continue;
        }

        if (code >= 400) {
            LOG_W("api returned %ld on %s %s: %s", code, method, path, resp.data ? resp.data : "");
        }

        if (out)
            *out = resp;
        else
            empty_buffer(&resp);
        return (code);
    }
    return (429);
}

CN_API long client_api_request(Client *client, const char *method, const char *path, const char *body)
{
    return (api_request(client, method, path, body, NULL));
}


CN_API uint8_t client_api_call(Client *client, const char *method, cJSON *body, Buffer *out, const char *fmt, ...)
{
    char path[512];
    char *json = NULL;
    long code;
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(path, sizeof(path), fmt, ap);
    va_end(ap);

    if (n < 0 || (size_t)n >= sizeof(path)) {
        cJSON_Delete(body);
        RAISE(ERR_OUT_OF_BOUND, "api path too long.");
        return (1);
    }

    if (body) {
        json = cJSON_PrintUnformatted(body);
        cJSON_Delete(body);

        if (!json) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate request json.");
            return (1);
        }
    }

    code = api_request(client, method, path, json, out);
    free(json);

    if (code < 0) {
        PROPAGATE_ERR();
        return (1);
    }

    if (code == 429 || code >= 400) {
        if (out)
            empty_buffer(out);

        if (code == 429) {
            RAISE(ERR_RUNTIME, "rate limited.");
        } else {
            RAISE_FMT(ERR_RUNTIME, "api refused %s %s (http %ld).", method, path, code);
        }
        return (1);
    }

    return (0);
}
