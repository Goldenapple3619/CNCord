#ifndef _PARSING_H_
    #define _PARSING_H_

    #include "client.h"

    cnbool json_flag(const cJSON *obj, const char *key);
    uint64_t json_u64(const cJSON *obj, const char *key);
    int json_int(const cJSON *obj, const char *key);
    char *json_str(const cJSON *obj, const char *key);

    uint8_t parse_user(User *out, const cJSON *user, const cJSON *member);
    uint8_t parse_role(Role *out, const cJSON *role);
    uint8_t parse_emoji(Emoji *out, const cJSON *emoji);
    uint8_t parse_attachment(Attachment *out, const cJSON *attachment);
    uint8_t parse_embed(Embed *out, const cJSON *embed);
    uint8_t parse_channel(Channel *out, const cJSON *item);
    uint8_t parse_interaction(Interaction *out, const cJSON *d);
    uint8_t parse_message(Message *out, const cJSON *d);
    uint8_t parse_member(Member *out, const cJSON *member);

    uint8_t load_roles(Guild *guild, const cJSON *array);
    uint8_t load_emojis(Guild *guild, const cJSON *array);
    uint8_t load_channels(Guild *guild, const cJSON *array);
    uint8_t load_members(Guild *guild, const cJSON *array);
    uint8_t load_attachments(Message *msg, const cJSON *array);
    uint8_t load_embeds(Message *msg, const cJSON *array);
    uint8_t load_embed_fields(Embed *embed, const cJSON *array);
    uint8_t load_mentions(Message *msg, const cJSON *array);
    uint8_t load_strings(struct generic_vector_s *vec, const cJSON *array);
#endif