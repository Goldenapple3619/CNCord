#ifndef _TYPES_H_
    #define _TYPES_H_

    #include <stdlib.h>

    #include "core.h"
    #include "generic_vector.h"

    #define MAX_COMMAND_OPTIONS 25
    #define EMBED_MAX_FIELDS 25
    #define EMBED_MAX_PER_MESSAGE 10

    typedef enum {
        CHAN_TEXT = 0,
        CHAN_DM = 1,
        CHAN_VOICE = 2,
        CHAN_GROUP_DM = 3,
        CHAN_CATEGORY = 4,
        CHAN_ANNOUNCEMENT = 5,
        CHAN_ANNOUNCEMENT_THREAD = 10,
        CHAN_PUBLIC_THREAD = 11,
        CHAN_PRIVATE_THREAD = 12,
        CHAN_STAGE_VOICE = 13,
        CHAN_DIRECTORY = 14,
        CHAN_FORUM = 15,
        CHAN_MEDIA = 16
    } ChannelType;

    typedef enum {
        CMD_OPT_STRING = 3,
        CMD_OPT_INTEGER = 4,
        CMD_OPT_BOOLEAN = 5,
        CMD_OPT_USER = 6,
        CMD_OPT_CHANNEL = 7,
        CMD_OPT_ROLE = 8,
        CMD_OPT_MENTIONABLE = 9,
        CMD_OPT_NUMBER = 10
    } CommandOptionType;

    typedef enum {
        INTERACTION_PING = 1,
        INTERACTION_COMMAND = 2,
        INTERACTION_COMPONENT = 3,
        INTERACTION_AUTOCOMPLETE = 4,
        INTERACTION_MODAL_SUBMIT = 5
    } InteractionType;

    typedef struct {
        char *id;
        char *name;
        cnbool animated;
    } Emoji;

    typedef struct {
        char *id;
        char *filename;
        char *url;
        char *content_type;
        uint64_t size;
    } Attachment;

    typedef struct {
        char *name;
        char *value;
        cnbool is_inline;
    } EmbedField;

    typedef struct {
        char *type;
        char *title;
        char *description;
        char *url;
        char *timestamp; /* ISO8601 */
        char *footer_text;
        char *footer_icon_url;
        char *author_name;
        char *author_url;
        char *author_icon_url;
        char *image_url;
        char *thumbnail_url;
        uint32_t color; /* 0xRRGGBB */

        cnbool has_color;

        struct generic_vector_s fields;
    } Embed;

    typedef struct {
        char *id;
        char *name;
        uint64_t color;
        uint64_t permissions;
        int32_t position;

        cnbool hoist;
        cnbool managed;
        cnbool mentionable;
    } Role;

    typedef struct {
        char *id;
        char *username;
        char *display_name;
        char *avatar;

        cnbool is_bot;
        cnbool is_system;
    } User;

    typedef struct {
        char *id;
        char *nick;
        char *joined_at;
        User user;

        struct generic_vector_s roles;
    } Member;

    typedef struct {
        char *id;
        char *content;
        char *channel_id;
        char *guild_id;
        char *timestamp;
        char *edited_timestamp;
        char *reply_to_id;
        User author;

        int type;
        cnbool pinned;
        cnbool mention_everyone;

        struct generic_vector_s mentions;
        struct generic_vector_s mention_roles;
        struct generic_vector_s attachments;
        struct generic_vector_s embeds;
    } Message;

    typedef struct {
        char *id;
        char *name;
        char *topic;
        char *parent_id;
        char *owner_id;
        char *last_pin_timestamp;
        ChannelType type;
        int32_t position;

        cnbool nsfw;
        cnbool archived;
        cnbool locked;

        struct generic_vector_s messages;
    } Channel;

    typedef struct {
        char *id;
        char *name;
        char *icon;
        char *owner_id;
        char *description;
        uint64_t member_count;

        struct generic_vector_s roles;
        struct generic_vector_s emojis;
        struct generic_vector_s channels;
        struct generic_vector_s members;
    } Guild;

    typedef struct {
        char *user_id;
        char *message_id;
        char *channel_id;
        char *guild_id;
        Emoji emoji;
    } Reaction;

    typedef struct {
        char *code;
        char *channel_id;
        char *guild_id;
        uint64_t max_age;
        uint64_t max_uses;
        cnbool temporary;
    } Invite;

    typedef struct {
        char *user_id;
        char *guild_id;
        char *channel_id;
        char *session_id;

        cnbool deaf;
        cnbool mute;
        cnbool self_deaf;
        cnbool self_mute;
        cnbool self_stream;
        cnbool self_video;
        cnbool suppress;
    } VoiceState;

    typedef struct {
        char *id;
        char *guild_id;
        char *channel_id;
        char *topic;
        int privacy_level;
    } StageInstance;

    typedef struct {
        char *id;
        char *guild_id;
        char *channel_id;
        char *creator_id;
        char *name;
        char *description;
        char *start_time;
        char *end_time;
        int status;
        int entity_type;
    } ScheduledEvent;


    typedef struct {
        const char *name;
        const char *description;

        CommandOptionType type;

        cnbool required;
    } CommandOption;

    typedef struct {
        char *name;

        CommandOptionType type;

        union {
            char *string;
            int64_t integer;
            double number;
            cnbool boolean;
        } as;
    } InteractionOption;

    typedef struct {
        char *id;
        char *application_id;
        char *token;
        char *guild_id;
        char *channel_id;
        char *command_name;
        User user;

        int type;

        size_t option_count;
        InteractionOption options[MAX_COMMAND_OPTIONS];
    } Interaction;

    CN_API const InteractionOption *get_interaction_option(const Interaction *interaction, const char *name);

    CN_API uint8_t emoji_copy(Emoji *dest, const Emoji *src);
    CN_API Emoji *emoji_dup(const Emoji *src);
    CN_API void emoji_empty(Emoji *emoji);
    CN_API void emoji_dedup(Emoji *emoji);

    CN_API uint8_t attachment_copy(Attachment *dest, const Attachment *src);
    CN_API Attachment *attachment_dup(const Attachment *src);
    CN_API void attachment_empty(Attachment *attachment);
    CN_API void attachment_dedup(Attachment *attachment);

    CN_API uint8_t embed_field_copy(EmbedField *dest, const EmbedField *src);
    CN_API EmbedField *embed_field_dup(const EmbedField *src);
    CN_API void embed_field_empty(EmbedField *field);
    CN_API void embed_field_dedup(EmbedField *field);

    CN_API uint8_t embed_copy(Embed *dest, const Embed *src);
    CN_API Embed *embed_dup(const Embed *src);
    CN_API void embed_empty(Embed *embed);
    CN_API void embed_dedup(Embed *embed);
    CN_API uint8_t embed_set(char **slot, const char *value);
    CN_API uint8_t embed_setf(char **slot, const char *fmt, ...);
    CN_API uint8_t embed_add_field(Embed *embed, const char *name, const char *value, cnbool is_inline);

    CN_API uint8_t role_copy(Role *dest, const Role *src);
    CN_API Role *role_dup(const Role *src);
    CN_API void role_empty(Role *role);
    CN_API void role_dedup(Role *role);

    CN_API uint8_t user_copy(User *dest, const User *src);
    CN_API User *user_dup(const User *src);
    CN_API void user_empty(User *user);
    CN_API void user_dedup(User *user);

    CN_API uint8_t member_copy(Member *dest, const Member *src);
    CN_API Member *member_dup(const Member *src);
    CN_API void member_empty(Member *member);
    CN_API void member_dedup(Member *member);

    CN_API uint8_t message_copy(Message *dest, const Message *src);
    CN_API Message *message_dup(const Message *src);
    CN_API void message_empty(Message *message);
    CN_API void message_dedup(Message *message);

    CN_API uint8_t channel_copy(Channel *dest, const Channel *src);
    CN_API Channel *channel_dup(const Channel *src);
    CN_API void channel_empty(Channel *channel);
    CN_API void channel_dedup(Channel *channel);

    CN_API uint8_t guild_copy(Guild *dest, const Guild *src);
    CN_API Guild *guild_dup(const Guild *src);
    CN_API void guild_empty(Guild *guild);
    CN_API void guild_dedup(Guild *guild);
#endif