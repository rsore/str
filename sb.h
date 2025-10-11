/**
 *        _           _
 *  ___  | |__       | |__
 * / __| | '_ \      | '_ \
 * \__ \ | |_) |  _  | | | |
 * |___/ |_.__/  (_) |_| |_|
 *
 *  sb.h - v1.0.0
 *
 *  This file is placed in the public domain.
 *  See end of file for license details.
 *
 **/


#ifndef SB_H_
#define SB_H_

// SBDEF is prefixed to all function signatures and can be defined by the user.
// A common use-case is '#define SBDEF static inline' when sb.h is used in a
// single translation unit.
#ifndef SBDEF
#define SBDEF
#endif


#if !defined(SB_REALLOC) && !defined(SB_FREE)
#include <stdlib.h>
#endif

#ifndef SB_REALLOC
#define SB_REALLOC(ptr, new_size) realloc((ptr), (new_size))
#endif

#ifndef SB_FREE
#define SB_FREE(ptr) free((ptr))
#endif



#ifndef SB_START_SIZE
#define SB_START_SIZE 64u
#endif

#ifndef SB_EXP_GROWTH_FACTOR
#define SB_EXP_GROWTH_FACTOR 2 // Exponential growth factor before threshold
#endif

#ifndef SB_LIN_THRESHOLD
#define SB_LIN_THRESHOLD (1u * 1024u * 1024u) // 1 MB before switching from growth factor to linear growth
#endif

#ifndef SB_LIN_GROWTH_FACTOR
#define SB_LIN_GROWTH_FACTOR (256u * 1024u) // 256 KB after threshold
#endif



#if defined(__cplusplus) &&  __cplusplus >= 201703L && !defined(SB_IGNORE_NODISCARD)
#define SB_NODISCARD [[nodiscard]]
#else
#define SB_NODISCARD
#endif

#ifndef __cplusplus
#define SB_NO_PARAMS void
#else
#define SB_NO_PARAMS
#endif

#ifdef __cplusplus
#define SB_NOEXCEPT noexcept
#else
#define SB_NOEXCEPT
#endif

#ifdef __cplusplus
#define SB_NULL nullptr
#else
#define SB_NULL NULL
#endif



#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    char  *buffer;
    size_t capacity;
    size_t size;
} StringBuilder;


// Initialize an empty string builder
SB_NODISCARD SBDEF StringBuilder sb_init(SB_NO_PARAMS) SB_NOEXCEPT;

// Free builder buffer and reset
SBDEF void sb_free(StringBuilder *sb) SB_NOEXCEPT;

// Reset builder without deallocating
SBDEF void sb_reset(StringBuilder *sb) SB_NOEXCEPT;

// Notify an intent to append data. Will if necessary allocate memory
// to fit at least new_cap characters.
SB_NODISCARD SBDEF bool sb_reserve(StringBuilder *sb, size_t new_cap) SB_NOEXCEPT;

// Append one sized string to builder
SB_NODISCARD SBDEF bool sb_append_one_n(StringBuilder *sb, const char *str, size_t len) SB_NOEXCEPT;
// Append one NULL-terminated string to builder
SB_NODISCARD SBDEF bool sb_append_one(StringBuilder *sb, const char *str) SB_NOEXCEPT;

// Append several NULL-terminated strings to builder.
#define sb_append(sb_ptr, ...) sb_append_((sb_ptr), __VA_ARGS__, SB_NULL)
SB_NODISCARD SBDEF bool sb_append_(StringBuilder *sb, const char *new_data1, ...) SB_NOEXCEPT;

// Append one character to builder
SB_NODISCARD SBDEF bool sb_append_char(StringBuilder *sb, char c) SB_NOEXCEPT;

// Append one string builder's content to another's. 'app' is appended to 'sb'.
SB_NODISCARD SBDEF bool sb_append_sb(StringBuilder *sb, StringBuilder *app) SB_NOEXCEPT;

// Append a formatted string to builder. Formatting follows sprintf semantics.
SB_NODISCARD SBDEF bool sb_appendf(StringBuilder *sb, const char *fmt, ...) SB_NOEXCEPT;

// Allocate and return a null-terminated string of the contents of the builder. Caller must free with SB_FREE()
SB_NODISCARD SBDEF char *sb_to_cstr(StringBuilder *sb) SB_NOEXCEPT;

#ifdef __cplusplus
} // extern "C"
#endif



#ifdef SB_IMPLEMENTATION

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

SBDEF StringBuilder
sb_init(SB_NO_PARAMS) SB_NOEXCEPT
{
    StringBuilder result = {SB_NULL, 0, 0};

    return result;
}

SBDEF void
sb_free(StringBuilder *sb) SB_NOEXCEPT
{
    SB_FREE(sb->buffer);
    sb->buffer   = SB_NULL;
    sb->capacity = 0;
    sb->size     = 0;
}

SBDEF void
sb_reset(StringBuilder *sb) SB_NOEXCEPT
{
    sb->size = 0;
    sb->buffer[0] = '\0';
}

static inline bool
sb_grow_to_fit_(StringBuilder *sb, size_t n) SB_NOEXCEPT
{
    size_t np1 = n + 1; // 'n plus 1' to account for trailing NULL-terminator

    if (np1 <= sb->capacity) return true;

    size_t new_cap = sb->capacity ? sb->capacity : SB_START_SIZE;

    // Exponential growth until threshold
    while (new_cap < np1 && new_cap < SB_LIN_THRESHOLD) new_cap *= SB_EXP_GROWTH_FACTOR;

    // Linear growth after threshold
    while (new_cap < np1) new_cap += SB_LIN_GROWTH_FACTOR;

    void *new_buffer = SB_REALLOC(sb->buffer, new_cap);
    if (!new_buffer) {
        sb_free(sb);
        return false;
    }

    sb->buffer = (char *)new_buffer;
    sb->capacity = new_cap;

    sb->buffer[sb->size] = '\0';

    return true;
}

SBDEF bool
sb_reserve(StringBuilder *sb, size_t new_cap) SB_NOEXCEPT
{
    if (!sb_grow_to_fit_(sb, new_cap)) return false;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_append_one_n(StringBuilder *sb, const char *str, size_t len) SB_NOEXCEPT
{
    if (!sb_grow_to_fit_(sb, sb->size + len)) return false;

    memcpy(sb->buffer+sb->size, str, len);
    sb->size += len;
    sb->buffer[sb->size] = '\0';

    return true;
}

SBDEF bool
sb_append_one(StringBuilder *sb, const char *str) SB_NOEXCEPT
{
    size_t len = strlen(str);
    return sb_append_one_n(sb, str, len);
}

SBDEF bool
sb_append_(StringBuilder *sb, const char *new_data1, ...) SB_NOEXCEPT
{
    va_list args;
    va_start(args, new_data1);

    bool ok = true;
    const char *data = new_data1;
    while (ok && data != SB_NULL) {
        if (!sb_append_one(sb, data)) ok = false;
        data = va_arg(args, const char *);
    }

    va_end(args);

    return ok;
}

SBDEF bool
sb_append_char(StringBuilder *sb, char c) SB_NOEXCEPT
{
    if (!sb_grow_to_fit_(sb, sb->size + 1)) return false;
    sb->buffer[sb->size++] = c;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_append_sb(StringBuilder *sb, StringBuilder *app) SB_NOEXCEPT
{
    if (app->size) return sb_append_one_n(sb, app->buffer, app->size);
    return true;
}

SBDEF bool
sb_appendf(StringBuilder *sb, const char *fmt, ...) SB_NOEXCEPT
{
    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(SB_NULL, 0, fmt, args);
    va_end(args);
    if (len < 0) {
        sb_free(sb);
        return false;
    }
    size_t len_z = len + 1;

    if (!sb_grow_to_fit_(sb, sb->size + len_z)) return false;

    va_start(args, fmt);
    vsnprintf(sb->buffer + sb->size, len_z, fmt, args);
    va_end(args);

    sb->size += len;

    sb->buffer[sb->size] = '\0';

    return true;
}

SBDEF char *
sb_to_cstr(StringBuilder *sb) SB_NOEXCEPT
{
    if (sb->buffer == SB_NULL || sb->size == 0) {
        char *z = (char *)SB_REALLOC(SB_NULL, 1);
        if (!z) return SB_NULL;
        *z = '\0';
        return z;
    }

    char *buf = (char *)SB_REALLOC(SB_NULL, sb->size + 1);
    if (!buf) return SB_NULL;

    memcpy(buf, sb->buffer, sb->size);
    buf[sb->size] = '\0';

    return buf;
}

#endif // SB_IMPLEMENTATION

#endif // SB_H_



/*
  LICENSE

  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
*/
