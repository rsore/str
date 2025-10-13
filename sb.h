/**
 *        _           _
 *  ___  | |__       | |__
 * / __| | '_ \      | '_ \
 * \__ \ | |_) |  _  | | | |
 * |___/ |_.__/  (_) |_| |_|
 *
 *  sb.h - v1.0.1
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
    char  *buffer;   // allocated bytes; always NUL-terminated at buffer[size] when buffer != NULL
    size_t capacity; // total allocated bytes including space for NUL (>= size + 1 when buffer != NULL)
    size_t size;     // number of content bytes, excluding terminating NUL
} StringBuilder;


// Initialize an empty string builder
SB_NODISCARD SBDEF StringBuilder sb_init(SB_NO_PARAMS) SB_NOEXCEPT;

// Clone a builder by copying the data of src into dst
SB_NODISCARD SBDEF bool sb_clone(const StringBuilder *src, StringBuilder *dst) SB_NOEXCEPT;

// Move the contents of a builder to another. 'from' gets reset.
SB_NODISCARD SBDEF StringBuilder sb_move(StringBuilder *from) SB_NOEXCEPT;

// Copy the string into a new buffer.
// Returns a NULL-terminated string. Sets *out_len to the string length (excluding NULL) if not NULL.
// Caller must free the returned string using SB_FREE().
SB_NODISCARD SBDEF char *sb_strdup(const StringBuilder *sb, size_t *out_len) SB_NOEXCEPT;

// Release ownership of the internal buffer without shrinking.
// The builder is reset. Returns the internal buffer, NULL-terminated.
// Sets *out_len to the string length (excluding NUL) if not NULL.
// Caller must free the returned string using SB_FREE().
SB_NODISCARD SBDEF char *sb_release(StringBuilder *sb, size_t *out_len) SB_NOEXCEPT;

// Shrink the buffer to exactly size+1 before releasing ownership.
// The builder is reset. Returns the new buffer.
// Sets *out_len to the string length (excluding NUL) if not NULL.
// Caller must free the returned string using SB_FREE().
SB_NODISCARD SBDEF char *sb_shrink_and_release(StringBuilder *sb, size_t *out_len) SB_NOEXCEPT;

// Shrink internal buffer to fit size+1, to account for trailing
// NULL-terminator.
SB_NODISCARD SBDEF bool sb_shrink_to_fit(StringBuilder *sb) SB_NOEXCEPT;

// Reset builder without deallocating
SBDEF void sb_reset(StringBuilder *sb) SB_NOEXCEPT;

// Free builder buffer and reset
SBDEF void sb_free(StringBuilder *sb) SB_NOEXCEPT;

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
SB_NODISCARD SBDEF bool sb_append_sb(StringBuilder *sb, const StringBuilder *app) SB_NOEXCEPT;

// Append a formatted string to builder. Formatting follows sprintf semantics.
SB_NODISCARD SBDEF bool sb_appendf(StringBuilder *sb, const char *fmt, ...) SB_NOEXCEPT;

#ifdef __cplusplus
} // extern "C"
#endif



#ifdef SB_IMPLEMENTATION

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

SBDEF StringBuilder
sb_init(SB_NO_PARAMS) SB_NOEXCEPT
{
    StringBuilder result = {SB_NULL, 0, 0};

    // Best-effort: allocate a 1-byte buffer for the empty string.
    char *p = (char *)SB_REALLOC(SB_NULL, 1);
    if (p) {
        p[0] = '\0';
        result.buffer = p;
        result.capacity = 1; // total allocated bytes (size + 1)
        result.size = 0;
    }

    return result;
}

SBDEF bool
sb_clone(const StringBuilder *src, StringBuilder *dst) SB_NOEXCEPT
{
    if (src == dst) return true;

    if (!dst) return false;

    sb_reset(dst);

    if (!src) return true;
    if (src->size == 0) return true;

    if (!sb_reserve(dst, src->size)) return false;
    memcpy(dst->buffer, src->buffer, src->size + 1); // include '\0'
    dst->size = src->size;
    return true;
}

SBDEF StringBuilder
sb_move(StringBuilder *from) SB_NOEXCEPT
{
    if (!from) {
        return sb_init();
    }

    StringBuilder result = *from;

    *from = sb_init(); // best-effort re-init to valid empty state
    return result;
}

SBDEF char *
sb_strdup(const StringBuilder *sb, size_t *out_len) SB_NOEXCEPT
{
    size_t len = (sb && sb->buffer) ? sb->size : 0;

    if (len + 1 < len) return SB_NULL; // Overflow protection

    char *buf = (char *)SB_REALLOC(SB_NULL, len + 1);
    if (!buf) return SB_NULL;

    if (len) memcpy(buf, sb->buffer, len);
    buf[len] = '\0';

    if (out_len) *out_len = len;
    return buf;
}

SBDEF char *
sb_release(StringBuilder *sb, size_t *out_len) SB_NOEXCEPT
{
    if (!sb) return SB_NULL;

    if (!sb->buffer) {
        // Produce a fresh empty string if we never had a buffer (e.g., OOM on init)
        char *z = (char *)SB_REALLOC(SB_NULL, 1);
        if (!z) return SB_NULL;
        z[0] = '\0';
        if (out_len) *out_len = 0;
        return z;
    }

    // Ensure terminator invariant
    sb->buffer[sb->size] = '\0';

    if (out_len) *out_len = sb->size;
    char *result = sb->buffer;

    // Detach without freeing
    sb->buffer   = SB_NULL;
    sb->capacity = 0;
    sb->size     = 0;

    return result;
}

SBDEF char *
sb_shrink_and_release(StringBuilder *sb, size_t *out_len) SB_NOEXCEPT
{
    if (!sb) return SB_NULL;

    if (!sb->buffer) {
        char *z = (char *)SB_REALLOC(SB_NULL, 1);
        if (!z) return SB_NULL;
        z[0] = '\0';
        if (out_len) *out_len = 0;
        return z;
    }

    size_t need = sb->size + 1;
    if (need < sb->size) return SB_NULL; // overflow

    void *p = SB_REALLOC(sb->buffer, need);
    if (p) {
        sb->buffer   = (char *)p;
        sb->capacity = need;             // total bytes including NUL
        sb->buffer[sb->size] = '\0';
    } else {
        // If shrinking failed, keep existing buffer/capacity and still release.
        sb->buffer[sb->size] = '\0';
    }

    if (out_len) *out_len = sb->size;
    char *result = sb->buffer;

    // Detach
    sb->buffer   = SB_NULL;
    sb->capacity = 0;
    sb->size     = 0;

    return result;
}

SBDEF bool
sb_shrink_to_fit(StringBuilder *sb) SB_NOEXCEPT
{
    if (!sb) return false;
    if (!sb->buffer) return true;

    // already tight? capacity is total bytes including NUL
    if (sb->capacity == sb->size + 1) {
        sb->buffer[sb->size] = '\0';
        return true;
    }

    size_t need = sb->size + 1;
    if (need < sb->size) return false; // Overflow protection

    void *p = SB_REALLOC(sb->buffer, need);
    if (!p) return false;

    sb->buffer   = (char *)p;
    sb->capacity = need;               // total bytes including NUL
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF void
sb_reset(StringBuilder *sb) SB_NOEXCEPT
{
    if (!sb) return;

    sb->size = 0;

    if (sb->buffer) {
        sb->buffer[0] = '\0';
        // keep existing capacity/buffer
    } else {
        // Best-effort (e.g., if moved-from or OOM earlier)
        char *p = (char *)SB_REALLOC(SB_NULL, 1);
        if (p) {
            p[0] = '\0';
            sb->buffer = p;
            sb->capacity = 1;
            sb->size = 0;
        }
    }
}

SBDEF void
sb_free(StringBuilder *sb) SB_NOEXCEPT
{
    if (!sb) return;

    SB_FREE(sb->buffer);
    sb->buffer   = SB_NULL;
    sb->capacity = 0;
    sb->size     = 0;
}

static inline bool
sb_grow_to_fit_(StringBuilder *sb, size_t n) SB_NOEXCEPT
{
    if (!sb) return false;

    size_t np1 = n + 1; // 'n plus 1' to account for trailing NULL-terminator
    if (np1 < n) return false; // Overflow protection

    if (sb->buffer && np1 <= sb->capacity) {
        return true;
    }

    size_t new_cap = sb->capacity ? sb->capacity : (SB_START_SIZE > 1 ? SB_START_SIZE : 1);

    // Exponential growth until threshold
    while (new_cap < np1 && new_cap < SB_LIN_THRESHOLD) {
        if (new_cap > SIZE_MAX / SB_EXP_GROWTH_FACTOR) { new_cap = SIZE_MAX; break; } // Overflow protection
        new_cap *= SB_EXP_GROWTH_FACTOR;
    }

    // Linear growth after threshold
    while (new_cap < np1) {
        if (new_cap > SIZE_MAX - SB_LIN_GROWTH_FACTOR) { new_cap = SIZE_MAX; break; } // Overflow protection
        new_cap += SB_LIN_GROWTH_FACTOR;
    }

    void *new_buffer = SB_REALLOC(sb->buffer, new_cap);
    if (!new_buffer) {
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
    if (!sb) return false;
    if (!sb_grow_to_fit_(sb, new_cap)) return false;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_append_one_n(StringBuilder *sb, const char *str, size_t len) SB_NOEXCEPT
{
    if (!sb || (!str && len)) return false;

    if (!sb_grow_to_fit_(sb, sb->size + len)) return false;

    if (len) memcpy(sb->buffer + sb->size, str, len);
    sb->size += len;
    sb->buffer[sb->size] = '\0';

    return true;
}

SBDEF bool
sb_append_one(StringBuilder *sb, const char *str) SB_NOEXCEPT
{
    if (!sb || !str) return false;
    size_t len = strlen(str);
    return sb_append_one_n(sb, str, len);
}

SBDEF bool
sb_append_(StringBuilder *sb, const char *new_data1, ...) SB_NOEXCEPT
{
    if (!sb) return false;

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
    if (!sb) return false;
    if (!sb_grow_to_fit_(sb, sb->size + 1)) return false;
    sb->buffer[sb->size++] = c;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_append_sb(StringBuilder *sb, const StringBuilder *app) SB_NOEXCEPT
{
    if (!sb) return false;
    if (!app || app->size == 0) return true;
    return sb_append_one_n(sb, app->buffer, app->size);
}

SBDEF bool
sb_appendf(StringBuilder *sb, const char *fmt, ...) SB_NOEXCEPT
{
    if (!sb || !fmt) return false;

    va_list args;

    va_start(args, fmt);
    int len = vsnprintf(SB_NULL, 0, fmt, args);
    va_end(args);
    if (len < 0) {
        return false;
    }

    if (!sb_grow_to_fit_(sb, sb->size + (size_t)len)) return false;

    va_start(args, fmt);
    vsnprintf(sb->buffer + sb->size, (size_t)len + 1, fmt, args);
    va_end(args);

    sb->size += (size_t)len;

    sb->buffer[sb->size] = '\0';

    return true;
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
