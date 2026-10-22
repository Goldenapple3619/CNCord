#include "client.h"
#include "error.h"
#include "log.h"
#include "event.h"
#include "parsing.h"
#include "cache.h"

#include <cJSON.h>

uint8_t on_interaction_create(Client *client, const cJSON *d)
{
    Interaction interaction = {0};

    if (parse_interaction(&interaction, d)) {
        PROPAGATE_ERR();
        return (1);
    }

    LOG_D("<INTERACTION id='%s' type=%d command='%s' user='%s'>",
        interaction.id, interaction.type,
        interaction.command_name ? interaction.command_name : "?",
        interaction.user.username ? interaction.user.username : "?");

    if (interaction.type == INTERACTION_COMMAND && client_dispatch_command(client, &interaction)) {
        PROPAGATE_ERR();
        return (1);
    }

    FIRE(client, on_interaction_create, &interaction);
    return (0);
}
