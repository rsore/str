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

#if (defined(__cplusplus) && __cplusplus < 201103L) || \
    (!defined(__cplusplus) && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L))
#error "sb.h requires at least C99 or C++11. Please use a newer compiler."
#endif


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

#if defined(__clang__) || defined(__GNUC__)
#  define SB_FMT(archetype, idx, first) __attribute__((format(archetype, idx, first)))
#else
#  define SB_FMT(archetype, idx, first)
#endif


#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    char  *buffer;   // allocated bytes; always NUL-terminated at buffer[size] when buffer != SB_NULL
    size_t capacity; // total allocated bytes including space for NUL (>= size + 1 when buffer != SB_NULL)
    size_t size;     // number of content bytes, excluding terminating NUL
} StringBuilder;



//
// Lifecycle
//

// Initialize an empty string builder
SB_NODISCARD SBDEF StringBuilder sb_init(SB_NO_PARAMS) SB_NOEXCEPT;

// Clone a builder by copying the data of src into dst
SB_NODISCARD SBDEF bool sb_clone(const StringBuilder *src, StringBuilder *dst) SB_NOEXCEPT;

// Move the contents of a builder to another. 'from' gets cleared.
SB_NODISCARD SBDEF StringBuilder sb_move(StringBuilder *from) SB_NOEXCEPT;

// Clear builder without deallocating
SBDEF void sb_clear(StringBuilder *sb) SB_NOEXCEPT;

// Free builder buffer and clear
SBDEF void sb_free(StringBuilder *sb) SB_NOEXCEPT;


//
// Ownership of raw buffer
//

// Copy the string into a new buffer.
// Returns a NUL-terminated string. Sets *out_len to the string length (excluding NUL) if not SB_NULL.
// Caller must free the returned string using SB_FREE().
SB_NODISCARD SBDEF char *sb_strdup(const StringBuilder *sb, size_t *out_len) SB_NOEXCEPT;

// Release ownership of the internal buffer without shrinking.
// The builder is cleared. Returns the internal buffer, NUL-terminated.
// Sets *out_len to the string length (excluding NUL) if not SB_NULL.
// Caller must free the returned string using SB_FREE().
SB_NODISCARD SBDEF char *sb_release(StringBuilder *sb, size_t *out_len) SB_NOEXCEPT;

// Shrink the buffer to exactly size+1 before releasing ownership.
// The builder is cleared. Returns the new buffer.
// Sets *out_len to the string length (excluding NUL) if not SB_NULL.
// Caller must free the returned string using SB_FREE().
SB_NODISCARD SBDEF char *sb_shrink_and_release(StringBuilder *sb, size_t *out_len) SB_NOEXCEPT;


//
// Capacity management
//

// Shrink internal buffer to fit size+1, to account for trailing
// NUL-terminator.
SB_NODISCARD SBDEF bool sb_shrink_to_fit(StringBuilder *sb) SB_NOEXCEPT;

// Reserve space for at least new_len content bytes (excluding the trailing NUL)
SB_NODISCARD SBDEF bool sb_reserve(StringBuilder *sb, size_t new_len) SB_NOEXCEPT;


//
// Append
//

// Append one sized string to builder
SB_NODISCARD SBDEF bool sb_append_one_n(StringBuilder *sb, const char *str, size_t len) SB_NOEXCEPT;
// Append one NUL-terminated string to builder
SB_NODISCARD SBDEF bool sb_append_one(StringBuilder *sb, const char *str) SB_NOEXCEPT;

// Append several NUL-terminated strings to builder.
#define sb_append(sb_ptr, ...) sb_append_((sb_ptr), __VA_ARGS__, (const char *)SB_NULL)
SB_NODISCARD SBDEF bool sb_append_(StringBuilder *sb, const char *new_data1, ...) SB_NOEXCEPT;

// Append one character to builder
SB_NODISCARD SBDEF bool sb_append_char(StringBuilder *sb, char c) SB_NOEXCEPT;

// Append the same character n times
SB_NODISCARD SBDEF bool sb_append_repeat(StringBuilder *sb, char c, size_t n) SB_NOEXCEPT;

// Append one string builder's content to another's. 'app' is appended to 'sb'.
SB_NODISCARD SBDEF bool sb_append_sb(StringBuilder *sb, const StringBuilder *app) SB_NOEXCEPT;

// vprintf-style append
SB_NODISCARD SBDEF bool sb_vappendf(StringBuilder *sb, const char *fmt, va_list args) SB_NOEXCEPT  SB_FMT(printf, 2, 0);

// sprintf-style append
SB_NODISCARD SBDEF bool sb_appendf(StringBuilder *sb, const char *fmt, ...) SB_NOEXCEPT SB_FMT(printf, 2, 3);


//
// Edits
//

// Insert and erase
SB_NODISCARD SBDEF bool sb_insert_one_n(StringBuilder *sb, size_t pos, const char *str, size_t len) SB_NOEXCEPT;
SB_NODISCARD SBDEF bool sb_insert_one(StringBuilder *sb, size_t pos, const char *str) SB_NOEXCEPT;
SB_NODISCARD SBDEF bool sb_erase(StringBuilder *sb, size_t pos, size_t len) SB_NOEXCEPT;

// Replace a range with a string
SB_NODISCARD SBDEF bool sb_replace_one_n(StringBuilder *sb, size_t pos, size_t len, const char *str, size_t slen) SB_NOEXCEPT;
SB_NODISCARD SBDEF bool sb_replace_one(StringBuilder *sb, size_t pos, size_t len, const char *str) SB_NOEXCEPT;


//
// Inspection, trim, search
//

// Inspect last char without removing it. Returns false if empty
SB_NODISCARD SBDEF bool sb_back(const StringBuilder *sb, char *out_char) SB_NOEXCEPT;

// Pop last char. Returns false if empty
SB_NODISCARD SBDEF bool sb_pop_back(StringBuilder *sb, char *out_char) SB_NOEXCEPT;

// Trim helpers
SB_NODISCARD SBDEF bool sb_ltrim(StringBuilder *sb) SB_NOEXCEPT;
SB_NODISCARD SBDEF bool sb_rtrim(StringBuilder *sb) SB_NOEXCEPT;
SB_NODISCARD SBDEF bool sb_trim(StringBuilder *sb) SB_NOEXCEPT;

// Search for the first/last occurrence of needle.
// Returns SIZE_MAX if not found.
// If needle is empty, sb_find returns 0 and sb_rfind returns sb->size.
SB_NODISCARD SBDEF size_t sb_find(const StringBuilder *sb, const char *needle) SB_NOEXCEPT;
SB_NODISCARD SBDEF size_t sb_find_n(const StringBuilder *sb, const char *needle, size_t nlen) SB_NOEXCEPT;
SB_NODISCARD SBDEF size_t sb_rfind(const StringBuilder *sb, const char *needle) SB_NOEXCEPT;
SB_NODISCARD SBDEF size_t sb_rfind_n(const StringBuilder *sb, const char *needle, size_t nlen) SB_NOEXCEPT;


//
// File IO
//

SB_NODISCARD SBDEF bool sb_write_file(const StringBuilder *sb, FILE *f) SB_NOEXCEPT;
SB_NODISCARD SBDEF bool sb_read_file(StringBuilder *sb, FILE *f) SB_NOEXCEPT;


#ifdef __cplusplus
} // extern "C"
#endif



#ifdef SB_IMPLEMENTATION

#include <ctype.h>
#include <stdint.h>
#include <string.h>

SBDEF StringBuilder
sb_init(SB_NO_PARAMS) SB_NOEXCEPT
{
    StringBuilder result = {SB_NULL, 0, 0};

    char *p = (char *)SB_REALLOC(SB_NULL, 1);
    if (p) {
        p[0] = '\0';
        result.buffer = p;
        result.capacity = 1;
        result.size = 0;
    }

    return result;
}

SBDEF bool
sb_clone(const StringBuilder *src, StringBuilder *dst) SB_NOEXCEPT
{
    if (src == dst) return true;

    if (!dst) return false;

    sb_clear(dst);

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

    *from = sb_init();
    return result;
}

SBDEF void
sb_clear(StringBuilder *sb) SB_NOEXCEPT
{
    if (!sb) return;

    sb->size = 0;

    if (sb->buffer) {
        sb->buffer[0] = '\0';
    } else {
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
        char *z = (char *)SB_REALLOC(SB_NULL, 1);
        if (!z) return SB_NULL;
        z[0] = '\0';
        if (out_len) *out_len = 0;
        return z;
    }

    sb->buffer[sb->size] = '\0';

    if (out_len) *out_len = sb->size;
    char *result = sb->buffer;

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
    if (need < sb->size) return SB_NULL; // Overflow protection

    void *p = SB_REALLOC(sb->buffer, need);
    if (p) {
        sb->buffer   = (char *)p;
        sb->capacity = need;
        sb->buffer[sb->size] = '\0';
    } else {
        sb->buffer[sb->size] = '\0';
    }

    if (out_len) *out_len = sb->size;
    char *result = sb->buffer;

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

    if (sb->capacity == sb->size + 1) {
        sb->buffer[sb->size] = '\0';
        return true;
    }

    size_t need = sb->size + 1;
    if (need < sb->size) return false; // Overflow protection

    void *p = SB_REALLOC(sb->buffer, need);
    if (!p) return false;

    sb->buffer   = (char *)p;
    sb->capacity = need;
    sb->buffer[sb->size] = '\0';
    return true;
}

static inline bool
sb_would_overflow_(size_t a, size_t b)
{
    return b > SIZE_MAX - a;
}

static inline bool
sb_grow_to_fit_(StringBuilder *sb, size_t n) SB_NOEXCEPT
{
    if (!sb) return false;

    size_t np1 = n + 1; // 'n plus 1' to account for trailing NUL-terminator
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
sb_reserve(StringBuilder *sb, size_t new_len) SB_NOEXCEPT
{
    if (!sb) return false;
    if (!sb_grow_to_fit_(sb, new_len)) return false;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_append_one_n(StringBuilder *sb, const char *str, size_t len) SB_NOEXCEPT
{
    if (!sb || (!str && len)) return false;

    if (sb_would_overflow_(sb->size, len)) return false;
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
    if (sb_would_overflow_(sb->size, 1)) return false;
    if (!sb_grow_to_fit_(sb, sb->size + 1)) return false;
    sb->buffer[sb->size++] = c;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_append_repeat(StringBuilder *sb, char c, size_t n) SB_NOEXCEPT
{
    if (!sb) return false;
    if (n == 0) return true;
    if (sb_would_overflow_(sb->size, n)) return false;
    if (!sb_grow_to_fit_(sb, sb->size + n)) return false;

    memset(sb->buffer + sb->size, (unsigned char)c, n);
    sb->size += n;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_append_sb(StringBuilder *sb, const StringBuilder *app) SB_NOEXCEPT
{
    if (!sb) return false;
    if (!app || app->size == 0) return true;

    if (sb == app) {
        size_t len = sb->size;
        if (!sb_grow_to_fit_(sb, sb->size + len)) return false;
        // source and destination are the same allocation, so use memmove
        memmove(sb->buffer + sb->size, sb->buffer, len);
        sb->size += len;
        sb->buffer[sb->size] = '\0';
        return true;
    }

    return sb_append_one_n(sb, app->buffer, app->size);
}

SBDEF bool
sb_vappendf(StringBuilder *sb, const char *fmt, va_list args) SB_NOEXCEPT
{
    if (!sb || !fmt) return false;

    va_list ap;
    va_copy(ap, args);
    int need = vsnprintf(SB_NULL, 0, fmt, ap);
    va_end(ap);
    if (need < 0) return false;

    if (!sb_grow_to_fit_(sb, sb->size + (size_t)need)) return false;

    va_copy(ap, args);
    vsnprintf(sb->buffer + sb->size, (size_t)need + 1, fmt, ap);
    va_end(ap);

    sb->size += (size_t)need;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_appendf(StringBuilder *sb, const char *fmt, ...) SB_NOEXCEPT
{
    if (!sb || !fmt) return false;
    va_list args;
    va_start(args, fmt);
    bool ok = sb_vappendf(sb, fmt, args);
    va_end(args);
    return ok;
}

SBDEF bool
sb_insert_one_n(StringBuilder *sb, size_t pos, const char *str, size_t len) SB_NOEXCEPT
{
    if (!sb) return false;
    if (pos > sb->size) return false;
    if (!str && len) return false;
    if (len == 0) return true;

    if (sb_would_overflow_(sb->size, len)) return false;
    if (!sb_grow_to_fit_(sb, sb->size + len)) return false;

    size_t tail = sb->size - pos;
    memmove(sb->buffer + pos + len, sb->buffer + pos, tail);
    if (len) memcpy(sb->buffer + pos, str, len);

    sb->size += len;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_insert_one(StringBuilder *sb, size_t pos, const char *str) SB_NOEXCEPT
{
    if (!str) return false;
    size_t len = strlen(str);
    return sb_insert_one_n(sb, pos, str, len);
}

SBDEF bool
sb_erase(StringBuilder *sb, size_t pos, size_t len) SB_NOEXCEPT
{
    if (!sb) return false;
    if (pos > sb->size) return false;
    if (len == 0) return true;

    size_t remain = sb->size - pos;
    if (len > remain) len = remain;

    size_t end  = pos + len;
    size_t tail = sb->size - end;

    if (tail) memmove(sb->buffer + pos, sb->buffer + end, tail);

    sb->size -= len;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_replace_one_n(StringBuilder *sb, size_t pos, size_t len, const char *str, size_t slen) SB_NOEXCEPT
{
    if (!sb) return false;
    if (pos > sb->size) return false;
    if (!str && slen) return false;

    // Calculate target end after erase
    size_t end = pos + len;
    if (end > sb->size) end = sb->size;
    size_t cut = end - pos;

    // Resize to new total size
    size_t tmp = sb->size - cut;
    if (sb_would_overflow_(tmp, slen)) return false;
    size_t new_size = tmp + slen;
    if (!sb_grow_to_fit_(sb, new_size)) return false;

    // Move tail to its new position if needed
    size_t old_tail = sb->size - end;
    if (old_tail && (slen != cut)) {
        memmove(sb->buffer + pos + slen, sb->buffer + end, old_tail);
    }

    if (slen) memcpy(sb->buffer + pos, str, slen);

    sb->size = new_size;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_replace_one(StringBuilder *sb, size_t pos, size_t len, const char *str) SB_NOEXCEPT
{
    if (!str) return false;
    size_t slen = strlen(str);
    return sb_replace_one_n(sb, pos, len, str, slen);
}

SBDEF bool
sb_back(const StringBuilder *sb, char *out_char) SB_NOEXCEPT
{
    if (!sb || sb->size == 0) return false;
    if (out_char) *out_char = sb->buffer[sb->size - 1];
    return true;
}

SBDEF bool
sb_pop_back(StringBuilder *sb, char *out_char) SB_NOEXCEPT
{
    if (!sb || sb->size == 0) return false;
    if (out_char) *out_char = sb->buffer[sb->size - 1];
    sb->size -= 1;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_ltrim(StringBuilder *sb) SB_NOEXCEPT
{
    if (!sb || sb->size == 0) return true;

    size_t i = 0;
    while (i < sb->size && isspace((unsigned char)sb->buffer[i])) i++;

    if (i == 0) return true;
    size_t remain = sb->size - i;
    memmove(sb->buffer, sb->buffer + i, remain);
    sb->size = remain;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_rtrim(StringBuilder *sb) SB_NOEXCEPT
{
    if (!sb || sb->size == 0) return true;

    size_t i = sb->size;
    while (i > 0 && isspace((unsigned char)sb->buffer[i - 1])) i--;

    if (i == sb->size) return true;
    sb->size = i;
    sb->buffer[sb->size] = '\0';
    return true;
}

SBDEF bool
sb_trim(StringBuilder *sb) SB_NOEXCEPT
{
    if (!sb) return false;
    bool ok = sb_rtrim(sb);
    if (!ok) return false;
    return sb_ltrim(sb);
}

SBDEF size_t
sb_find_n(const StringBuilder *sb, const char *needle, size_t nlen) SB_NOEXCEPT
{
    if (!sb || !sb->buffer) return SIZE_MAX;
    if (!needle) return SIZE_MAX;
    if (nlen == 0) return 0;
    if (sb->size == 0 || nlen > sb->size) return SIZE_MAX;

    const char *hay = sb->buffer;
    size_t last = sb->size - nlen;

    for (size_t i = 0; i <= last; ++i) {
        if (hay[i] == needle[0] && memcmp(hay + i, needle, nlen) == 0) {
            return i;
        }
    }
    return SIZE_MAX;
}

SBDEF size_t
sb_find(const StringBuilder *sb, const char *needle) SB_NOEXCEPT
{
    if (!needle) return SIZE_MAX;
    return sb_find_n(sb, needle, strlen(needle));
}

SBDEF size_t
sb_rfind_n(const StringBuilder *sb, const char *needle, size_t nlen) SB_NOEXCEPT
{
    if (!sb || !sb->buffer) return SIZE_MAX;
    if (!needle) return SIZE_MAX;
    if (nlen == 0) return sb->size;
    if (sb->size == 0 || nlen > sb->size) return SIZE_MAX;

    const char *hay = sb->buffer;
    for (size_t i = sb->size - nlen + 1; i-- > 0; ) {
        if (hay[i] == needle[0] && memcmp(hay + i, needle, nlen) == 0) {
            return i;
        }
        if (i == 0) break;
    }
    return SIZE_MAX;
}

SBDEF size_t
sb_rfind(const StringBuilder *sb, const char *needle) SB_NOEXCEPT
{
    if (!needle) return SIZE_MAX;
    return sb_rfind_n(sb, needle, strlen(needle));
}

SBDEF bool
sb_write_file(const StringBuilder *sb, FILE *f) SB_NOEXCEPT
{
    if (!sb || !f) return false;
    if (sb->size == 0) return true;
    size_t n = fwrite(sb->buffer, 1, sb->size, f);
    return n == sb->size;
}

SBDEF bool
sb_read_file(StringBuilder *sb, FILE *f) SB_NOEXCEPT
{
    if (!sb || !f) return false;

    char buf[32768];
    for (;;) {
        size_t r = fread(buf, 1, sizeof(buf), f);
        if (r > 0) {
            if (!sb_append_one_n(sb, buf, r)) return false;
        }
        if (r < sizeof(buf)) {
            if (feof(f)) break;
            if (ferror(f)) return false;
        }
    }
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
