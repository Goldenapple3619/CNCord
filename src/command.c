#include "client.h"
#include "error.h"
#include "log.h"
#include "api.h"

#include <cJSON.h>

static cnbool find_command(Client *client, const char *name, Command *out)
{
    cnbool found = false;

    client_commands_lock(client);

    for (size_t i = 0; i < client->commands.size; ++i) {
        const Command *cmd = client->commands.content[i];

        if (!strcmp(cmd->name, name)) {
            *out = *cmd;
            found = true;
            break;
        }
    }

    client_commands_unlock(client);
    return (found);
}

static uint8_t check_command(const Command *command)
{
    cnbool seen_optional = false;

    if (!command->name || !*command->name || !command->description || !*command->description || !command->handler) {
        RAISE(ERR_INVALID_POINTER, "a command needs a name, a description and a handler.");
        return (1);
    }

    if (command->option_count > MAX_COMMAND_OPTIONS || (command->option_count && !command->options)) {
        RAISE_FMT(ERR_OUT_OF_BOUND, "command '%s' has an invalid option list (max %d).", command->name, MAX_COMMAND_OPTIONS);
        return (1);
    }

    for (size_t i = 0; i < command->option_count; ++i) {
        const CommandOption *opt = &command->options[i];

        if (!opt->name || !*opt->name || !opt->description || !*opt->description) {
            RAISE_FMT(ERR_INVALID_POINTER, "an option of command '%s' has no name or description.", command->name);
            return (1);
        }

        if (!opt->required) {
            seen_optional = true;
        } else if (seen_optional) {
            RAISE_FMT(ERR_NOT_COMPATIBLE, "command '%s': required options must come before optional ones.", command->name);
            return (1);
        }
    }
    return (0);
}

CN_API const InteractionOption *get_interaction_option(const Interaction *interaction, const char *name)
{
    if (!interaction || !name)
        return (NULL);

    for (size_t i = 0; i < interaction->option_count; ++i) {
        if (interaction->options[i].name && !strcmp(interaction->options[i].name, name))
            return (&interaction->options[i]);
    }
    return (NULL);
}

CN_API uint8_t client_register_command(Client *client, const Command *command)
{
    Command *copy;
    Command known;

    if (!client || !command) {
        RAISE(ERR_INVALID_POINTER, "can't register an empty command.");
        return (1);
    }

    if (check_command(command)) {
        PROPAGATE_ERR();
        return (1);
    }

    copy = (Command *)malloc(sizeof(Command));

    if (!copy) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate command.");
        return (1);
    }
    *copy = *command;

    if (find_command(client, command->name, &known)) {
        free(copy);
        RAISE_FMT(ERR_RUNTIME, "command '%s' is already registered.", command->name);
        return (1);
    }

    client_commands_lock(client);
    if (insert_generic_vector(&client->commands, copy)) {
        client_commands_unlock(client);
        free(copy);
        PROPAGATE_ERR();
        return (1);
    }
    client_commands_unlock(client);

    return (0);
}

uint8_t client_dispatch_command(Client *client, const Interaction *interaction)
{
    Command command;

    if (!interaction->command_name)
        return (0);

    if (!find_command(client, interaction->command_name, &command)) {
        LOG_W("no handler for command '%s'", interaction->command_name);

        if (client_interaction_reply(client, interaction, "unknown command.", true)) {
            PROPAGATE_ERR();
            return (1);
        }
        return (0);
    }

    if (command.handler(client, interaction)) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}
