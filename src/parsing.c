#include "core.h"
#include "error.h"
#include "client.h"
#include "parsing.h"

#include <cJSON.h>

char *json_str(const cJSON *obj, const char *key)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);

    return (cJSON_IsString(item) ? item->valuestring : NULL);
}

int json_int(const cJSON *obj, const char *key)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);

    return (cJSON_IsNumber(item) ? item->valueint : 0);
}

uint64_t json_u64(const cJSON *obj, const char *key)
{
    const cJSON *item = cJSON_GetObjectItemCaseSensitive(obj, key);

    return (cJSON_IsNumber(item) && item->valuedouble > 0 ? (uint64_t)item->valuedouble : 0);
}

cnbool json_flag(const cJSON *obj, const char *key)
{
    return (cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(obj, key)) ? true : false);
}

uint8_t parse_user(User *out, const cJSON *user, const cJSON *member)
{
    char *display_name = NULL;

    if (!cJSON_IsObject(user) || !json_str(user, "id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (cJSON_IsObject(member))
        display_name = json_str(member, "nick");
    if (!display_name)
        display_name = json_str(user, "global_name");

    out->id = json_str(user, "id");
    out->username = json_str(user, "username");
    out->display_name = display_name;
    out->avatar = json_str(user, "avatar");
    out->is_bot = json_flag(user, "bot");
    out->is_system = json_flag(user, "system");
    return (0);
}

uint8_t parse_role(Role *out, const cJSON *role)
{
    char *permissions = json_str(role, "permissions");

    if (!cJSON_IsObject(role) || !json_str(role, "id") || !json_str(role, "name")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    out->id = json_str(role, "id");
    out->name = json_str(role, "name");
    if (cJSON_IsObject(cJSON_GetObjectItemCaseSensitive(role, "colors")))
        out->color = json_u64(cJSON_GetObjectItemCaseSensitive(role, "colors"), "primary_color");
    else
        out->color = 0xffffff;
    out->permissions = permissions ? strtoull(permissions, NULL, 10) : 0;
    out->position = json_int(role, "position");
    out->hoist = json_flag(role, "hoist");
    out->managed = json_flag(role, "managed");
    out->mentionable = json_flag(role, "mentionable");
    return (0);
}

uint8_t parse_emoji(Emoji *out, const cJSON *emoji)
{
    if (!cJSON_IsObject(emoji) || !json_str(emoji, "id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    out->id = json_str(emoji, "id");
    out->name = json_str(emoji, "name");
    out->animated = json_flag(emoji, "animated");
    return (0);
}

uint8_t parse_attachment(Attachment *out, const cJSON *attachment)
{
    if (!cJSON_IsObject(attachment) || !json_str(attachment, "id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    out->id = json_str(attachment, "id");
    out->filename = json_str(attachment, "filename");
    out->url = json_str(attachment, "url");
    out->content_type = json_str(attachment, "content_type");
    out->size = json_u64(attachment, "size");
    return (0);
}

uint8_t parse_embed(Embed *out, const cJSON *embed)
{
    const cJSON *color = cJSON_GetObjectItemCaseSensitive(embed, "color");
    const cJSON *footer = cJSON_GetObjectItemCaseSensitive(embed, "footer");
    const cJSON *author = cJSON_GetObjectItemCaseSensitive(embed, "author");
    const cJSON *image = cJSON_GetObjectItemCaseSensitive(embed, "image");
    const cJSON *thumbnail = cJSON_GetObjectItemCaseSensitive(embed, "thumbnail");
    const cJSON *fields = cJSON_GetObjectItemCaseSensitive(embed, "fields");

    if (!cJSON_IsObject(embed)) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    out->type = json_str(embed, "type");
    out->title = json_str(embed, "title");
    out->description = json_str(embed, "description");
    out->url = json_str(embed, "url");
    out->timestamp = json_str(embed, "timestamp");
    out->footer_text = json_str(footer, "text");
    out->footer_icon_url = json_str(footer, "icon_url");
    out->author_name = json_str(author, "name");
    out->author_url = json_str(author, "url");
    out->author_icon_url = json_str(author, "icon_url");
    out->image_url = json_str(image, "url");
    out->thumbnail_url = json_str(thumbnail, "url");

    if (cJSON_IsNumber(color)) {
        out->color = (uint32_t)color->valuedouble;
        out->has_color = true;
    }

    if (cJSON_IsArray(fields) && load_embed_fields(out, fields)) {
        PROPAGATE_ERR();
        empty_generic_vector(&out->fields, &free);
        return (1);
    }
    return (0);
}

uint8_t parse_channel(Channel *out, const cJSON *item)
{
    const cJSON *type = cJSON_GetObjectItemCaseSensitive(item, "type");
    const cJSON *meta = cJSON_GetObjectItemCaseSensitive(item, "thread_metadata");

    if (!json_str(item, "id") || !cJSON_IsNumber(type)) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    out->id = json_str(item, "id");
    out->name = json_str(item, "name");
    out->topic = json_str(item, "topic");
    out->parent_id = json_str(item, "parent_id");
    out->owner_id = json_str(item, "owner_id");
    out->last_pin_timestamp = json_str(item, "last_pin_timestamp");
    out->type = type->valueint;
    out->position = json_int(item, "position");
    out->nsfw = json_flag(item, "nsfw");
    out->archived = json_flag(meta, "archived");
    out->locked = json_flag(meta, "locked");
    return (0);
}

uint8_t parse_interaction(Interaction *out, const cJSON *d)
{
    const cJSON *member = cJSON_GetObjectItemCaseSensitive(d, "member");
    const cJSON *user = cJSON_IsObject(member)
        ? cJSON_GetObjectItemCaseSensitive(member, "user")
        : cJSON_GetObjectItemCaseSensitive(d, "user");
    const cJSON *data = cJSON_GetObjectItemCaseSensitive(d, "data");
    const cJSON *channel = cJSON_GetObjectItemCaseSensitive(d, "channel");
    const cJSON *opt;
    const cJSON *value;
    InteractionOption *iopt;

    out->id = json_str(d, "id");
    out->application_id = json_str(d, "application_id");
    out->token = json_str(d, "token");
    out->guild_id = json_str(d, "guild_id");
    out->channel_id = json_str(d, "channel_id");
    out->command_name = json_str(data, "name");
    out->type = json_int(d, "type");

    if (!out->channel_id)
        out->channel_id = json_str(channel, "id");

    if (!out->id || !out->application_id || !out->token) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    if (parse_user(&out->user, user, member)) {
        PROPAGATE_ERR();
        return (1);
    }

    cJSON_ArrayForEach(opt, cJSON_GetObjectItemCaseSensitive(data, "options")) {
        value = cJSON_GetObjectItemCaseSensitive(opt, "value");

        if (out->option_count >= MAX_COMMAND_OPTIONS)
            break;

        iopt = &out->options[out->option_count];
        iopt->name = json_str(opt, "name");
        iopt->type = json_int(opt, "type");

        if (!iopt->name)
            continue;

        switch (iopt->type) {
        case CMD_OPT_STRING: case CMD_OPT_USER: case CMD_OPT_CHANNEL:
        case CMD_OPT_ROLE: case CMD_OPT_MENTIONABLE:
            iopt->as.string = cJSON_IsString(value) ? value->valuestring : NULL;
            break;
        case CMD_OPT_INTEGER:
            iopt->as.integer = (int64_t)(cJSON_IsNumber(value) ? value->valuedouble : 0);
            break;
        case CMD_OPT_NUMBER:
            iopt->as.number = cJSON_IsNumber(value) ? value->valuedouble : 0;
            break;
        case CMD_OPT_BOOLEAN:
            iopt->as.boolean = cJSON_IsTrue(value) ? true : false;
            break;
        default:
            continue;
        }
        out->option_count++;
    }
    return (0);
}

uint8_t parse_message(Message *out, const cJSON *d)
{
    const cJSON *author = cJSON_GetObjectItemCaseSensitive(d, "author");
    const cJSON *reference = cJSON_GetObjectItemCaseSensitive(d, "message_reference");
    const cJSON *mentions = cJSON_GetObjectItemCaseSensitive(d, "mentions");
    const cJSON *mention_roles = cJSON_GetObjectItemCaseSensitive(d, "mention_roles");
    const cJSON *attachments = cJSON_GetObjectItemCaseSensitive(d, "attachments");
    const cJSON *embeds = cJSON_GetObjectItemCaseSensitive(d, "embeds");

    if (!json_str(d, "id") || !json_str(d, "channel_id")) {
        RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
        return (1);
    }

    out->id = json_str(d, "id");
    out->content = json_str(d, "content");
    out->channel_id = json_str(d, "channel_id");
    out->guild_id = json_str(d, "guild_id");
    out->timestamp = json_str(d, "timestamp");
    out->edited_timestamp = json_str(d, "edited_timestamp");
    out->reply_to_id = json_str(reference, "message_id");
    out->type = json_int(d, "type");
    out->pinned = json_flag(d, "pinned");
    out->mention_everyone = json_flag(d, "mention_everyone");

    if (cJSON_IsObject(author) && parse_user(&out->author, author, cJSON_GetObjectItemCaseSensitive(d, "member"))) {
        PROPAGATE_ERR();
        return (1);
    }

    if ((cJSON_IsArray(mentions) && load_mentions(out, mentions))
        || (cJSON_IsArray(mention_roles) && load_strings(&out->mention_roles, mention_roles))
        || (cJSON_IsArray(attachments) && load_attachments(out, attachments))
        || (cJSON_IsArray(embeds) && load_embeds(out, embeds))) {
        PROPAGATE_ERR();
        return (1);
    }

    return (0);
}

uint8_t parse_member(Member *out, const cJSON *member)
{
    const cJSON *roles = cJSON_GetObjectItemCaseSensitive(member, "roles");

    if (parse_user(&out->user, cJSON_GetObjectItemCaseSensitive(member, "user"), member)) {
        PROPAGATE_ERR();
        return (1);
    }

    out->id = out->user.id;
    out->nick = json_str(member, "nick");
    out->joined_at = json_str(member, "joined_at");

    if (cJSON_IsArray(roles) && load_strings(&out->roles, roles)) {
        PROPAGATE_ERR();
        empty_generic_vector(&out->roles, &free);
        return (1);
    }

    return (0);
}

uint8_t load_roles(Guild *guild, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        Role role = {0};
        Role *copy;

        if (parse_role(&role, item)) {
            PROPAGATE_ERR();
            return (1);
        }

        copy = role_dup(&role);

        if (!copy) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&guild->roles, copy)) {
            PROPAGATE_ERR();
            role_dedup(copy);
            return (1);
        }
    }
    return (0);
}

uint8_t load_emojis(Guild *guild, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        Emoji emoji = {0};
        Emoji *copy;

        if (parse_emoji(&emoji, item)) {
            PROPAGATE_ERR();
            return (1);
        }

        copy = emoji_dup(&emoji);

        if (!copy) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&guild->emojis, copy)) {
            PROPAGATE_ERR();
            emoji_dedup(copy);
            return (1);
        }
    }
    return (0);
}

uint8_t load_channels(Guild *guild, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        Channel channel = {0};
        Channel *copy;

        if (parse_channel(&channel, item)) {
            PROPAGATE_ERR();
            return (1);
        }

        copy = channel_dup(&channel);

        if (!copy) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&guild->channels, copy)) {
            PROPAGATE_ERR();
            channel_dedup(copy);
            return (1);
        }
    }
    return (0);
}

uint8_t load_members(Guild *guild, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        Member member = {0};
        Member *copy;

        if (parse_member(&member, item)) {
            PROPAGATE_ERR();
            return (1);
        }

        copy = member_dup(&member);
        empty_generic_vector(&member.roles, &free);

        if (!copy) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&guild->members, copy)) {
            PROPAGATE_ERR();
            member_dedup(copy);
            return (1);
        }
    }
    return (0);
}

uint8_t load_strings(struct generic_vector_s *vec, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        char *copy;

        if (!cJSON_IsString(item)) {
            RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
            return (1);
        }

        copy = strdup(item->valuestring);

        if (!copy) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new string.");
            return (1);
        }

        if (insert_generic_vector(vec, copy)) {
            PROPAGATE_ERR();
            free(copy);
            return (1);
        }
    }
    return (0);
}

uint8_t load_mentions(Message *msg, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        User user = {0};
        User *copy;

        if (parse_user(&user, item, cJSON_GetObjectItemCaseSensitive(item, "member"))) {
            PROPAGATE_ERR();
            return (1);
        }

        copy = user_dup(&user);

        if (!copy) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&msg->mentions, copy)) {
            PROPAGATE_ERR();
            user_dedup(copy);
            return (1);
        }
    }
    return (0);
}

uint8_t load_attachments(Message *msg, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        Attachment attachment = {0};
        Attachment *copy;

        if (parse_attachment(&attachment, item)) {
            PROPAGATE_ERR();
            return (1);
        }

        copy = attachment_dup(&attachment);

        if (!copy) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&msg->attachments, copy)) {
            PROPAGATE_ERR();
            attachment_dedup(copy);
            return (1);
        }
    }
    return (0);
}

uint8_t load_embed_fields(Embed *embed, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        EmbedField *field;

        if (!cJSON_IsObject(item)) {
            RAISE(ERR_CORRUPT_OR_INVALID, "badly formed payload received.");
            return (1);
        }

        field = calloc(1, sizeof *field);

        if (!field) {
            RAISE(ERR_OUT_OF_MEMORY, "failed to allocate embed field.");
            return (1);
        }

        field->name = json_str(item, "name");
        field->value = json_str(item, "value");
        field->is_inline = json_flag(item, "inline");

        if (insert_generic_vector(&embed->fields, field)) {
            PROPAGATE_ERR();
            free(field);
            return (1);
        }
    }
    return (0);
}

uint8_t load_embeds(Message *msg, const cJSON *array)
{
    const cJSON *item;

    cJSON_ArrayForEach(item, array) {
        Embed embed = {0};
        Embed *copy;

        if (parse_embed(&embed, item)) {
            PROPAGATE_ERR();
            return (1);
        }

        copy = embed_dup(&embed);
        empty_generic_vector(&embed.fields, &free);

        if (!copy) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&msg->embeds, copy)) {
            PROPAGATE_ERR();
            embed_dedup(copy);
            return (1);
        }
    }
    return (0);
}