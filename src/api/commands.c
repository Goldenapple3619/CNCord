#include "client.h"
#include "api.h"
#include "log.h"
#include "error.h"

#include <cJSON.h>

static cJSON *command_to_json(const Command *command)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *options;

    if (!root)
        return (NULL);

    if (!cJSON_AddStringToObject(root, "name", command->name)
        || !cJSON_AddStringToObject(root, "description", command->description)
        || !cJSON_AddNumberToObject(root, "type", 1)) {
        cJSON_Delete(root);
        return (NULL);
    }

    if (!command->option_count)
        return (root);

    options = cJSON_AddArrayToObject(root, "options");

    if (!options) {
        cJSON_Delete(root);
        return (NULL);
    }

    for (size_t i = 0; i < command->option_count; ++i) {
        const CommandOption *opt = &command->options[i];
        cJSON *item = cJSON_CreateObject();

        if (!item) {
            cJSON_Delete(root);
            return (NULL);
        }

        cJSON_AddItemToArray(options, item);

        if (!cJSON_AddStringToObject(item, "name", opt->name)
            || !cJSON_AddStringToObject(item, "description", opt->description)
            || !cJSON_AddNumberToObject(item, "type", opt->type)
            || !cJSON_AddBoolToObject(item, "required", opt->required ? 1 : 0)) {
            cJSON_Delete(root);
            return (NULL);
        }
    }
    return (root);
}

CN_API uint8_t client_sync_commands(Client *client, const char *guild_id)
{
    const char *app_id;
    cJSON *array;
    long code;
    size_t count;

    if (!client) {
        RAISE(ERR_INVALID_POINTER, "can't sync commands of an empty client.");
        return (1);
    }

    app_id = client->application_id ? client->application_id : client->id;

    if (!app_id) {
        RAISE(ERR_RUNTIME, "can't sync commands before the client is ready.");
        return (1);
    }

    array = cJSON_CreateArray();

    if (!array) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate commands json.");
        return (1);
    }

    client_commands_lock(client);

    count = client->commands.size;
    for (size_t i = 0; i < count; ++i) {
        cJSON *item = command_to_json(client->commands.content[i]);

        if (!item) {
            client_commands_unlock(client);
            cJSON_Delete(array);
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate command json.");
            return (1);
        }
        cJSON_AddItemToArray(array, item);
    }

    client_commands_unlock(client);

    if (guild_id)
        code = client_api_call(client, "PUT", array, NULL, "/applications/%s/guilds/%s/commands", app_id, guild_id);
    else
        code = client_api_call(client, "PUT", array, NULL, "/applications/%s/commands", app_id);

    if (code) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_I("synced %zu slash command(s) %s", count, guild_id ? "to the guild" : "globally");
    return (0);
}
