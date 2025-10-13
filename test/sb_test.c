#define SBDEF static inline
#define SB_IGNORE_NODISCARD
#define SB_IMPLEMENTATION
#include "../sb.h"

#include "minitest.h"
#include <string.h>
#include <stdio.h>

MT_DEFINE_TEST(init)
{
    StringBuilder sb = sb_init();
    MT_CHECK_THAT(sb.size == 0);
    if (sb.buffer) {
        MT_CHECK_THAT(sb.capacity >= 1);
        MT_CHECK_THAT(sb.buffer[0] == '\0');
    } else {
        MT_CHECK_THAT(sb.capacity == 0);
    }
}

MT_DEFINE_TEST(reserve)
{
    StringBuilder sb = sb_init();

    sb_reserve(&sb, 16);
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_CHECK_THAT(sb.capacity >= 16 + 1);
    MT_CHECK_THAT(sb.size == 0);
    MT_CHECK_THAT(sb.buffer[0] == '\0');
}

MT_DEFINE_TEST(append_one)
{
    StringBuilder sb = sb_init();

    sb_append_one(&sb, "Hello");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 5 + 1);
    MT_ASSERT_THAT(sb.size == 5);

    MT_CHECK_THAT(memcmp(sb.buffer, "Hello", 5) == 0);

    sb_append_one(&sb, " world");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 11 + 1);
    MT_ASSERT_THAT(sb.size == 11);

    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world", 11) == 0);
    MT_CHECK_THAT(sb.buffer[sb.size] == '\0');
}

MT_DEFINE_TEST(append)
{
    StringBuilder sb = sb_init();

    sb_append(&sb, "Hello", " world");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 11 + 1);
    MT_ASSERT_THAT(sb.size == 11);

    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world", 11) == 0);
    MT_CHECK_THAT(sb.buffer[sb.size] == '\0');
}

MT_DEFINE_TEST(append_char)
{
    StringBuilder sb = sb_init();

    sb_append_char(&sb, 'a');
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 1 + 1);
    MT_ASSERT_THAT(sb.size == 1);
    MT_CHECK_THAT(sb.buffer[0] == 'a');
    MT_CHECK_THAT(sb.buffer[1] == '\0');

    sb_append_char(&sb, 'b');
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 2 + 1);
    MT_ASSERT_THAT(sb.size == 2);
    MT_CHECK_THAT(sb.buffer[0] == 'a');
    MT_CHECK_THAT(sb.buffer[1] == 'b');
    MT_CHECK_THAT(sb.buffer[2] == '\0');
}

MT_DEFINE_TEST(append_sb)
{
    StringBuilder sb1 = sb_init();
    StringBuilder sb2 = sb_init();

    sb_append(&sb1, "Hello", " world");
    sb_append(&sb2, "Foo", " bar", " ", "baz");

    sb_append_sb(&sb1, &sb2);
    MT_CHECK_THAT(memcmp(sb1.buffer, "Hello worldFoo bar baz", sb1.size) == 0);
    MT_CHECK_THAT(sb1.buffer[sb1.size] == '\0');
}

MT_DEFINE_TEST(appendf)
{
    StringBuilder sb = sb_init();

    sb_appendf(&sb, "Hello %s", "world");
    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world", sb.size) == 0);
    MT_CHECK_THAT(sb.buffer[sb.size] == '\0');

    sb_appendf(&sb, " %d + %d = %d", 34, 35, 69);
    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world 34 + 35 = 69", sb.size) == 0);
    MT_CHECK_THAT(sb.buffer[sb.size] == '\0');
}

static int call_vappendf_wrapper(StringBuilder *sb, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bool ok = sb_vappendf(sb, fmt, ap);
    va_end(ap);
    return ok ? 0 : 1;
}

MT_DEFINE_TEST(vappendf)
{
    StringBuilder sb = sb_init();

    MT_CHECK_THAT(call_vappendf_wrapper(&sb, "%s %d", "ok", 7) == 0);
    MT_CHECK_THAT(strcmp(sb.buffer, "ok 7") == 0);

    MT_CHECK_THAT(call_vappendf_wrapper(&sb, " %c%c", 'X', 'Y') == 0);
    MT_CHECK_THAT(strcmp(sb.buffer, "ok 7 XY") == 0);
}

MT_DEFINE_TEST(strdup)
{
    StringBuilder sb = sb_init();
    size_t len = 12345;
    char *cstr = sb_strdup(&sb, &len);
    MT_ASSERT_THAT(cstr != NULL);
    MT_CHECK_THAT(strcmp(cstr, "") == 0);
    MT_CHECK_THAT(len == strlen(cstr));
    SB_FREE(cstr);

    sb_append(&sb, "Hello", "world");
    cstr = sb_strdup(&sb, NULL);
    MT_CHECK_THAT(strcmp(cstr, "Helloworld") == 0);
    SB_FREE(cstr);
}

MT_DEFINE_TEST(release)
{
    StringBuilder sb = sb_init();
    sb_append(&sb, "Foo", "Bar");

    size_t len = 0;
    char *owned = sb_release(&sb, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "FooBar") == 0);
    MT_CHECK_THAT(len == 6);

    MT_CHECK_THAT(sb.buffer == NULL);
    MT_CHECK_THAT(sb.size == 0);
    MT_CHECK_THAT(sb.capacity == 0);

    SB_FREE(owned);

    sb = sb_init();
    len = 777;
    owned = sb_release(&sb, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "") == 0);
    MT_CHECK_THAT(len == 0);
    MT_CHECK_THAT(sb.buffer == NULL && sb.size == 0 && sb.capacity == 0);
    SB_FREE(owned);
}

MT_DEFINE_TEST(shrink_and_release)
{
    StringBuilder sb = sb_init();
    sb_reserve(&sb, 1024);
    sb_append_one(&sb, "xyz");

    size_t len = 0;
    char *owned = sb_shrink_and_release(&sb, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "xyz") == 0);
    MT_CHECK_THAT(len == 3);

    MT_CHECK_THAT(sb.buffer == NULL);
    MT_CHECK_THAT(sb.size == 0);
    MT_CHECK_THAT(sb.capacity == 0);

    SB_FREE(owned);

    sb = sb_init();
    owned = sb_shrink_and_release(&sb, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "") == 0);
    MT_CHECK_THAT(len == 0);
    MT_CHECK_THAT(sb.buffer == NULL && sb.size == 0 && sb.capacity == 0);
    SB_FREE(owned);
}

MT_DEFINE_TEST(append_repeat_and_pop_back)
{
    StringBuilder sb = sb_init();

    sb_append_repeat(&sb, 'a', 5);
    MT_CHECK_THAT(strcmp(sb.buffer, "aaaaa") == 0);

    char ch = 0;
    MT_CHECK_THAT(sb_back(&sb, &ch) == true);
    MT_CHECK_THAT(ch == 'a');

    MT_CHECK_THAT(sb_pop_back(&sb, &ch) == true);
    MT_CHECK_THAT(ch == 'a');
    MT_CHECK_THAT(strcmp(sb.buffer, "aaaa") == 0);

    MT_CHECK_THAT(sb_pop_back(&sb, NULL) == true);
    MT_CHECK_THAT(strcmp(sb.buffer, "aaa") == 0);
}

MT_DEFINE_TEST(trim_l_r)
{
    StringBuilder sb = sb_init();

    sb_append_one(&sb, " \t  hello  \n");
    sb_ltrim(&sb);
    MT_CHECK_THAT(strcmp(sb.buffer, "hello  \n") == 0);

    sb_rtrim(&sb);
    MT_CHECK_THAT(strcmp(sb.buffer, "hello") == 0);
}

MT_DEFINE_TEST(trim_both)
{
    StringBuilder sb = sb_init();

    sb_append_one(&sb, " \t  hi there \r\n");
    sb_trim(&sb);
    MT_CHECK_THAT(strcmp(sb.buffer, "hi there") == 0);
}

MT_DEFINE_TEST(insert_and_erase)
{
    StringBuilder sb = sb_init();
    sb_append_one(&sb, "HelloWorld");

    sb_insert_one(&sb, 5, " ");
    MT_CHECK_THAT(strcmp(sb.buffer, "Hello World") == 0);

    sb_insert_one_n(&sb, 6, "big ", 4);
    MT_CHECK_THAT(strcmp(sb.buffer, "Hello big World") == 0);

    sb_erase(&sb, 6, 4);
    MT_CHECK_THAT(strcmp(sb.buffer, "Hello World") == 0);

    sb_erase(&sb, 5, 1);
    MT_CHECK_THAT(strcmp(sb.buffer, "HelloWorld") == 0);
}

MT_DEFINE_TEST(replace_one)
{
    StringBuilder sb = sb_init();
    sb_append_one(&sb, "Hello brave new world");

    sb_replace_one(&sb, 6, 5, "small");
    MT_CHECK_THAT(strcmp(sb.buffer, "Hello small new world") == 0);

    sb_replace_one_n(&sb, 12, 3, "old", 3);
    MT_CHECK_THAT(strcmp(sb.buffer, "Hello small old world") == 0);

    sb_replace_one(&sb, 6, 5, "");
    MT_CHECK_THAT(strcmp(sb.buffer, "Hello  old world") == 0);
}

MT_DEFINE_TEST(find_and_rfind)
{
    StringBuilder sb = sb_init();
    sb_append_one(&sb, "one two two three two");

    size_t p = sb_find(&sb, "two");
    MT_CHECK_THAT(p == 4);

    p = sb_find_n(&sb, "two", 3);
    MT_CHECK_THAT(p == 4);

    p = sb_find(&sb, "zzz");
    MT_CHECK_THAT(p == (size_t)-1);

    p = sb_rfind(&sb, "two");
    MT_CHECK_THAT(p == 18);

    p = sb_rfind_n(&sb, "one", 3);
    MT_CHECK_THAT(p == 0);
}

MT_DEFINE_TEST(write_and_read_file)
{
    StringBuilder sb = sb_init();
    sb_append_one(&sb, "alpha\nbeta\ngamma");

    FILE *f = tmpfile();
    MT_ASSERT_THAT(f != NULL);

    MT_CHECK_THAT(sb_write_file(&sb, f) == true);

    rewind(f);

    StringBuilder rd = sb_init();
    MT_CHECK_THAT(sb_read_file(&rd, f) == true);
    MT_CHECK_THAT(strcmp(rd.buffer, "alpha\nbeta\ngamma") == 0);

    fclose(f);
    sb_free(&rd);
}

MT_DEFINE_TEST(free)
{
    StringBuilder sb = sb_init();

    sb_append(&sb, "Foo", "Bar", "Baz");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity > 0);
    MT_ASSERT_THAT(sb.size > 0);

    sb_free(&sb);
    MT_CHECK_THAT(sb.buffer == NULL);
    MT_CHECK_THAT(sb.capacity == 0);
    MT_CHECK_THAT(sb.size == 0);
}

MT_DEFINE_TEST(clear)
{
    StringBuilder sb = sb_init();

    sb_append(&sb, "Foo", "Bar", "Baz");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity > 0);
    MT_ASSERT_THAT(sb.size > 0);

    sb_clear(&sb);
    MT_CHECK_THAT(sb.buffer != NULL || true);
    MT_CHECK_THAT(sb.size == 0);
    if (sb.buffer) MT_CHECK_THAT(sb.buffer[0] == '\0');
}

MT_DEFINE_TEST(clone)
{
    StringBuilder src = sb_init();
    sb_append(&src, "Foo", "Bar", "Baz");

    StringBuilder dst = sb_init();
    sb_clone(&src, &dst);

    MT_ASSERT_THAT(src.buffer != NULL);
    MT_ASSERT_THAT(dst.buffer != NULL);
    MT_ASSERT_THAT(dst.size == src.size);

    MT_CHECK_THAT(strcmp(src.buffer, "FooBarBaz") == 0);
    MT_CHECK_THAT(strcmp(dst.buffer, "FooBarBaz") == 0);
}

MT_DEFINE_TEST(move)
{
    StringBuilder src = sb_init();
    sb_append(&src, "Foo", "Bar", "Baz");

    StringBuilder dst = sb_move(&src);

    MT_ASSERT_THAT(dst.buffer != NULL);
    MT_ASSERT_THAT(dst.size == 9);
    MT_CHECK_THAT(strcmp(dst.buffer, "FooBarBaz") == 0);

    MT_ASSERT_THAT(src.size == 0);
    if (src.buffer) {
        MT_CHECK_THAT(src.buffer[0] == '\0');
    } else {
        MT_CHECK_THAT(src.capacity == 0);
    }

    sb_free(&dst);
}

int
main(void)
{
    MT_INIT();

    MT_RUN_TEST(init);

    MT_RUN_TEST(reserve);

    MT_RUN_TEST(append_one);
    MT_RUN_TEST(append);
    MT_RUN_TEST(append_char);
    MT_RUN_TEST(append_sb);
    MT_RUN_TEST(appendf);
    MT_RUN_TEST(vappendf);

    MT_RUN_TEST(strdup);
    MT_RUN_TEST(release);
    MT_RUN_TEST(shrink_and_release);

    MT_RUN_TEST(append_repeat_and_pop_back);
    MT_RUN_TEST(trim_l_r);
    MT_RUN_TEST(trim_both);
    MT_RUN_TEST(insert_and_erase);
    MT_RUN_TEST(replace_one);
    MT_RUN_TEST(find_and_rfind);
    MT_RUN_TEST(write_and_read_file);

    MT_RUN_TEST(free);
    MT_RUN_TEST(clear);

    MT_RUN_TEST(clone);
    MT_RUN_TEST(move);

    MT_PRINT_SUMMARY();
    return MT_EXIT_CODE;
}
