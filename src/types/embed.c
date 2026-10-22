#include "types.h"
#include "error.h"

#include <stdarg.h>
#include <stdio.h>

CN_API uint8_t embed_field_copy(EmbedField *dest, const EmbedField *src)
{
    embed_field_empty(dest);

    dest->is_inline = src->is_inline;

    dest->name = src->name ? strdup(src->name) : NULL;
    dest->value = src->value ? strdup(src->value) : NULL;

    if (src->name && !dest->name) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new name.");
        return (1);
    }
    if (src->value && !dest->value) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new value.");
        return (1);
    }

    return (0);
}

CN_API EmbedField *embed_field_dup(const EmbedField *src)
{
    EmbedField *dest = malloc(sizeof(EmbedField));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new embed field.");
        return (NULL);
    }

    memset(dest, 0, sizeof(EmbedField));

    if (embed_field_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)embed_field_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void embed_field_empty(EmbedField *field)
{
    if (field->name) {
        (void)free(field->name);
        field->name = NULL;
    }
    if (field->value) {
        (void)free(field->value);
        field->value = NULL;
    }
    field->is_inline = false;
}

CN_API void embed_field_dedup(EmbedField *field)
{
    embed_field_empty(field);
    (void)free(field);
}

CN_API uint8_t embed_copy(Embed *dest, const Embed *src)
{
    EmbedField *temp_field;

    embed_empty(dest);

    dest->color = src->color;
    dest->has_color = src->has_color;

    dest->type = src->type ? strdup(src->type) : NULL;
    dest->title = src->title ? strdup(src->title) : NULL;
    dest->description = src->description ? strdup(src->description) : NULL;
    dest->url = src->url ? strdup(src->url) : NULL;
    dest->timestamp = src->timestamp ? strdup(src->timestamp) : NULL;
    dest->footer_text = src->footer_text ? strdup(src->footer_text) : NULL;
    dest->footer_icon_url = src->footer_icon_url ? strdup(src->footer_icon_url) : NULL;
    dest->author_name = src->author_name ? strdup(src->author_name) : NULL;
    dest->author_url = src->author_url ? strdup(src->author_url) : NULL;
    dest->author_icon_url = src->author_icon_url ? strdup(src->author_icon_url) : NULL;
    dest->image_url = src->image_url ? strdup(src->image_url) : NULL;
    dest->thumbnail_url = src->thumbnail_url ? strdup(src->thumbnail_url) : NULL;

    if (src->type && !dest->type) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new type.");
        return (1);
    }
    if (src->title && !dest->title) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new title.");
        return (1);
    }
    if (src->description && !dest->description) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new description.");
        return (1);
    }
    if (src->url && !dest->url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new url.");
        return (1);
    }
    if (src->timestamp && !dest->timestamp) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new timestamp.");
        return (1);
    }
    if (src->footer_text && !dest->footer_text) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new footer_text.");
        return (1);
    }
    if (src->footer_icon_url && !dest->footer_icon_url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new footer_icon_url.");
        return (1);
    }
    if (src->author_name && !dest->author_name) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new author_name.");
        return (1);
    }
    if (src->author_url && !dest->author_url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new author_url.");
        return (1);
    }
    if (src->author_icon_url && !dest->author_icon_url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new author_icon_url.");
        return (1);
    }
    if (src->image_url && !dest->image_url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new image_url.");
        return (1);
    }
    if (src->thumbnail_url && !dest->thumbnail_url) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new thumbnail_url.");
        return (1);
    }

    for (size_t i = 0; i < src->fields.size; ++i) {
        temp_field = embed_field_dup(src->fields.content[i]);

        if (!temp_field) {
            PROPAGATE_ERR();
            return (1);
        }

        if (insert_generic_vector(&dest->fields, temp_field)) {
            PROPAGATE_ERR();
            embed_field_dedup(temp_field);
            return (1);
        }
    }

    return (0);
}

CN_API Embed *embed_dup(const Embed *src)
{
    Embed *dest = malloc(sizeof(Embed));

    if (!dest) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new embed.");
        return (NULL);
    }

    memset(dest, 0, sizeof(Embed));

    if (embed_copy(dest, src)) {
        PROPAGATE_ERR();
        (void)embed_dedup(dest);
        return (NULL);
    }

    return (dest);
}

CN_API void embed_empty(Embed *embed)
{
    empty_generic_vector(&embed->fields, (void (*)(void *))&embed_field_dedup);

    if (embed->type) {
        (void)free(embed->type);
        embed->type = NULL;
    }
    if (embed->title) {
        (void)free(embed->title);
        embed->title = NULL;
    }
    if (embed->description) {
        (void)free(embed->description);
        embed->description = NULL;
    }
    if (embed->url) {
        (void)free(embed->url);
        embed->url = NULL;
    }
    if (embed->timestamp) {
        (void)free(embed->timestamp);
        embed->timestamp = NULL;
    }
    if (embed->footer_text) {
        (void)free(embed->footer_text);
        embed->footer_text = NULL;
    }
    if (embed->footer_icon_url) {
        (void)free(embed->footer_icon_url);
        embed->footer_icon_url = NULL;
    }
    if (embed->author_name) {
        (void)free(embed->author_name);
        embed->author_name = NULL;
    }
    if (embed->author_url) {
        (void)free(embed->author_url);
        embed->author_url = NULL;
    }
    if (embed->author_icon_url) {
        (void)free(embed->author_icon_url);
        embed->author_icon_url = NULL;
    }
    if (embed->image_url) {
        (void)free(embed->image_url);
        embed->image_url = NULL;
    }
    if (embed->thumbnail_url) {
        (void)free(embed->thumbnail_url);
        embed->thumbnail_url = NULL;
    }
    embed->color = 0;
    embed->has_color = false;
}

CN_API void embed_dedup(Embed *embed)
{
    embed_empty(embed);
    (void)free(embed);
}

CN_API uint8_t embed_set(char **slot, const char *value)
{
    char *copy = value ? strdup(value) : NULL;

    if (value && !copy) {
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new embed string.");
        return (1);
    }

    (void)free(*slot);
    *slot = copy;
    return (0);
}

CN_API uint8_t embed_setf(char **slot, const char *fmt, ...)
{
    va_list ap;
    va_list cp;
    char *buf;
    int n;

    va_start(ap, fmt);
    va_copy(cp, ap);
    n = vsnprintf(NULL, 0, fmt, cp);
    va_end(cp);

    buf = n < 0 ? NULL : malloc((size_t)n + 1);

    if (!buf) {
        va_end(ap);
        RAISE(ERR_OUT_OF_MEMORY, "failed to allocate new embed string.");
        return (1);
    }

    vsnprintf(buf, (size_t)n + 1, fmt, ap);
    va_end(ap);

    (void)free(*slot);
    *slot = buf;
    return (0);
}

CN_API uint8_t embed_add_field(Embed *embed, const char *name, const char *value, cnbool is_inline)
{
    EmbedField src = {0};
    EmbedField *field;

    if (embed->fields.size >= EMBED_MAX_FIELDS) {
        RAISE(ERR_OUT_OF_BOUND, "an embed can't have more than 25 fields.");
        return (1);
    }

    src.name = (char *)name;
    src.value = (char *)value;
    src.is_inline = is_inline;

    field = embed_field_dup(&src);

    if (!field) {
        PROPAGATE_ERR();
        return (1);
    }

    if (insert_generic_vector(&embed->fields, field)) {
        PROPAGATE_ERR();
        embed_field_dedup(field);
        return (1);
    }
    return (0);
}