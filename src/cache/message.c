#include "client.h"
#include "cache.h"

uint8_t add_message_unlocked(Channel *channel, Message *msg, Message **dropped)
{
    struct generic_vector_s *messages = &channel->messages;

    if (insert_generic_vector(messages, msg)) {
        PROPAGATE_ERR();
        return (1);
    }

    if (messages->size > MAX_CACHED_MESSAGES) {
        *dropped = messages->content[0];
        remove_generic_ordered_vector(messages, 0, NULL);
    }
    return (0);
}