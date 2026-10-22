#ifndef _CACHE_H_
    #define _CACHE_H_

    #include "client.h"

    #define MAX_CACHED_MESSAGES 200
    #define NOT_FOUND ((size_t)-1)

    Channel *find_guild_channel_unlocked(Client *client, const char *guild_id, const char *channel_id);
    Channel *find_channel_unlocked(Client *client, const char *guild_id, const char *channel_id);
    Channel *get_dm_channel_unlocked(Client *client, const Message *msg);
    uint8_t remove_channel_unlocked(Client *client, const char *guild_id, const char *channel_id, Channel **old);
    uint8_t put_channel_unlocked(Client *client, const char *guild_id, Channel *fresh, Channel **old);

    Guild *find_guild_unlocked(Client *client, const char *id);
    uint8_t put_guild_unlocked(Client *client, Guild *fresh, Guild **old);
    uint8_t add_message_unlocked(Channel *channel, Message *msg, Message **dropped);

    Channel *find_channel_in(const struct generic_vector_s *channels, const char *id);
    void *take_by_id(struct generic_vector_s *vec, const char *id);
    size_t find_by_id(const struct generic_vector_s *vec, const char *id);
    uint8_t put_by_id(struct generic_vector_s *vec, void *fresh, const char *id, void **old);
    void swap_str(char **a, char **b);
    void swap_vec(struct generic_vector_s *a, struct generic_vector_s *b);

    void swap_guild_info_unlocked(Guild *cached, Guild *fresh);
#endif
