#define STRDEF static inline
#define STR_IGNORE_NODISCARD
#define STR_IMPLEMENTATION
#include "../str.h"

#include "minitest.h"
#include <string.h>
#include <stdio.h>

MT_DEFINE_TEST(init)
{
    String str = str_init();
    MT_CHECK_THAT(str.size == 0);
    if (str.buffer) {
        MT_CHECK_THAT(str.capacity >= 1);
        MT_CHECK_THAT(str.buffer[0] == '\0');
    } else {
        MT_CHECK_THAT(str.capacity == 0);
    }
    str_free(&str);
}

MT_DEFINE_TEST(reserve)
{
    String str = str_init();

    str_reserve(&str, 16);
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_CHECK_THAT(str.capacity >= 16 + 1);
    MT_CHECK_THAT(str.size == 0);
    MT_CHECK_THAT(str.buffer[0] == '\0');

    str_free(&str);
}

MT_DEFINE_TEST(append_one)
{
    String str = str_init();

    str_append_one(&str, "Hello");
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_ASSERT_THAT(str.capacity >= 5 + 1);
    MT_ASSERT_THAT(str.size == 5);

    MT_CHECK_THAT(memcmp(str.buffer, "Hello", 5) == 0);

    str_append_one(&str, " world");
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_ASSERT_THAT(str.capacity >= 11 + 1);
    MT_ASSERT_THAT(str.size == 11);

    MT_CHECK_THAT(memcmp(str.buffer, "Hello world", 11) == 0);
    MT_CHECK_THAT(str.buffer[str.size] == '\0');

    str_free(&str);
}

MT_DEFINE_TEST(append)
{
    String str = str_init();

    str_append(&str, "Hello", " world");
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_ASSERT_THAT(str.capacity >= 11 + 1);
    MT_ASSERT_THAT(str.size == 11);

    MT_CHECK_THAT(memcmp(str.buffer, "Hello world", 11) == 0);
    MT_CHECK_THAT(str.buffer[str.size] == '\0');

    str_free(&str);
}

MT_DEFINE_TEST(append_char)
{
    String str = str_init();

    str_append_char(&str, 'a');
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_ASSERT_THAT(str.capacity >= 1 + 1);
    MT_ASSERT_THAT(str.size == 1);
    MT_CHECK_THAT(str.buffer[0] == 'a');
    MT_CHECK_THAT(str.buffer[1] == '\0');

    str_append_char(&str, 'b');
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_ASSERT_THAT(str.capacity >= 2 + 1);
    MT_ASSERT_THAT(str.size == 2);
    MT_CHECK_THAT(str.buffer[0] == 'a');
    MT_CHECK_THAT(str.buffer[1] == 'b');
    MT_CHECK_THAT(str.buffer[2] == '\0');

    str_free(&str);
}

MT_DEFINE_TEST(append_str)
{
    String str1 = str_init();
    String str2 = str_init();

    str_append(&str1, "Hello", " world");
    str_append(&str2, "Foo", " bar", " ", "baz");

    str_append_str(&str1, &str2);
    MT_CHECK_THAT(memcmp(str1.buffer, "Hello worldFoo bar baz", str1.size) == 0);
    MT_CHECK_THAT(str1.buffer[str1.size] == '\0');

    str_append_str(&str1, &str1); // Self-append
    MT_CHECK_THAT(memcmp(str1.buffer, "Hello worldFoo bar bazHello worldFoo bar baz", str1.size) == 0);

    str_free(&str1);
    str_free(&str2);
}

MT_DEFINE_TEST(appendf)
{
    String str = str_init();

    str_appendf(&str, "Hello %s", "world");
    MT_CHECK_THAT(memcmp(str.buffer, "Hello world", str.size) == 0);
    MT_CHECK_THAT(str.buffer[str.size] == '\0');

    str_appendf(&str, " %d + %d = %d", 34, 35, 69);
    MT_CHECK_THAT(memcmp(str.buffer, "Hello world 34 + 35 = 69", str.size) == 0);
    MT_CHECK_THAT(str.buffer[str.size] == '\0');

    str_free(&str);
}

static int
call_vappendf_wrapper(String *str, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bool ok = str_vappendf(str, fmt, ap);
    va_end(ap);
    return ok ? 0 : 1;
}

MT_DEFINE_TEST(vappendf)
{
    String str = str_init();

    MT_CHECK_THAT(call_vappendf_wrapper(&str, "%s %d", "ok", 7) == 0);
    MT_CHECK_THAT(strcmp(str.buffer, "ok 7") == 0);

    MT_CHECK_THAT(call_vappendf_wrapper(&str, " %c%c", 'X', 'Y') == 0);
    MT_CHECK_THAT(strcmp(str.buffer, "ok 7 XY") == 0);

    str_free(&str);
}

MT_DEFINE_TEST(strdup)
{
    String str = str_init();
    size_t len = 12345;
    char *cstr = str_strdup(&str, &len);
    MT_ASSERT_THAT(cstr != NULL);
    MT_CHECK_THAT(strcmp(cstr, "") == 0);
    MT_CHECK_THAT(len == strlen(cstr));
    STR_FREE(cstr);

    str_append(&str, "Hello", "world");
    cstr = str_strdup(&str, NULL);
    MT_CHECK_THAT(strcmp(cstr, "Helloworld") == 0);
    STR_FREE(cstr);

    str_free(&str);
}

MT_DEFINE_TEST(release)
{
    String str = str_init();
    str_append(&str, "Foo", "Bar");

    size_t len = 0;
    char *owned = str_release(&str, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "FooBar") == 0);
    MT_CHECK_THAT(len == 6);

    MT_CHECK_THAT(str.buffer == NULL);
    MT_CHECK_THAT(str.size == 0);
    MT_CHECK_THAT(str.capacity == 0);

    STR_FREE(owned);

    str = str_init();
    len = 777;
    owned = str_release(&str, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "") == 0);
    MT_CHECK_THAT(len == 0);
    MT_CHECK_THAT(str.buffer == NULL && str.size == 0 && str.capacity == 0);
    STR_FREE(owned);

    str_free(&str);
}

MT_DEFINE_TEST(shrink_and_release)
{
    String str = str_init();
    str_reserve(&str, 1024);
    str_append_one(&str, "xyz");

    size_t len = 0;
    char *owned = str_shrink_and_release(&str, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "xyz") == 0);
    MT_CHECK_THAT(len == 3);

    MT_CHECK_THAT(str.buffer == NULL);
    MT_CHECK_THAT(str.size == 0);
    MT_CHECK_THAT(str.capacity == 0);

    STR_FREE(owned);

    str = str_init();
    owned = str_shrink_and_release(&str, &len);
    MT_ASSERT_THAT(owned != NULL);
    MT_CHECK_THAT(strcmp(owned, "") == 0);
    MT_CHECK_THAT(len == 0);
    MT_CHECK_THAT(str.buffer == NULL && str.size == 0 && str.capacity == 0);
    STR_FREE(owned);

    str_free(&str);
}

MT_DEFINE_TEST(append_repeat_and_pop_back)
{
    String str = str_init();

    str_append_repeat(&str, 'a', 5);
    MT_CHECK_THAT(strcmp(str.buffer, "aaaaa") == 0);

    char ch = 0;
    MT_CHECK_THAT(str_back(&str, &ch) == true);
    MT_CHECK_THAT(ch == 'a');

    MT_CHECK_THAT(str_pop_back(&str, &ch) == true);
    MT_CHECK_THAT(ch == 'a');
    MT_CHECK_THAT(strcmp(str.buffer, "aaaa") == 0);

    MT_CHECK_THAT(str_pop_back(&str, NULL) == true);
    MT_CHECK_THAT(strcmp(str.buffer, "aaa") == 0);

    str_free(&str);
}

MT_DEFINE_TEST(trim_l_r)
{
    String str = str_init();

    str_append_one(&str, " \t  hello  \n");
    str_ltrim(&str);
    MT_CHECK_THAT(strcmp(str.buffer, "hello  \n") == 0);

    str_rtrim(&str);
    MT_CHECK_THAT(strcmp(str.buffer, "hello") == 0);

    str_free(&str);
}

MT_DEFINE_TEST(trim_both)
{
    String str = str_init();

    str_append_one(&str, " \t  hi there \r\n");
    str_trim(&str);
    MT_CHECK_THAT(strcmp(str.buffer, "hi there") == 0);

    str_free(&str);
}

MT_DEFINE_TEST(insert_and_erase)
{
    String str = str_init();
    str_append_one(&str, "HelloWorld");

    str_insert_one(&str, 5, " ");
    MT_CHECK_THAT(strcmp(str.buffer, "Hello World") == 0);

    str_insert_one_n(&str, 6, "big ", 4);
    MT_CHECK_THAT(strcmp(str.buffer, "Hello big World") == 0);

    str_erase(&str, 6, 4);
    MT_CHECK_THAT(strcmp(str.buffer, "Hello World") == 0);

    str_erase(&str, 5, 1);
    MT_CHECK_THAT(strcmp(str.buffer, "HelloWorld") == 0);

    str_free(&str);
}

MT_DEFINE_TEST(replace_one)
{
    String str = str_init();
    str_append_one(&str, "Hello brave new world");

    str_replace_one(&str, 6, 5, "small");
    MT_CHECK_THAT(strcmp(str.buffer, "Hello small new world") == 0);

    str_replace_one_n(&str, 12, 3, "old", 3);
    MT_CHECK_THAT(strcmp(str.buffer, "Hello small old world") == 0);

    str_replace_one(&str, 6, 5, "");
    MT_CHECK_THAT(strcmp(str.buffer, "Hello  old world") == 0);

    str_free(&str);
}

MT_DEFINE_TEST(find_and_rfind)
{
    String str = str_init();
    str_append_one(&str, "one two two three two");

    size_t p = str_find(&str, "two");
    MT_CHECK_THAT(p == 4);

    p = str_find_n(&str, "two", 3);
    MT_CHECK_THAT(p == 4);

    p = str_find(&str, "zzz");
    MT_CHECK_THAT(p == SIZE_MAX);

    p = str_rfind(&str, "two");
    MT_CHECK_THAT(p == 18);

    p = str_rfind_n(&str, "one", 3);
    MT_CHECK_THAT(p == 0);

    str_free(&str);
}

MT_DEFINE_TEST(equals)
{
    String a = str_init();
    String b = str_init();

    MT_CHECK_THAT(str_equals(&a, &b) == true);
    MT_CHECK_THAT(str_equals(&a, &a) == true);

    str_append_one(&a, "hello");
    str_append_one(&b, "hello");
    MT_CHECK_THAT(str_equals(&a, &b) == true);

    String c = str_init();
    str_append_one(&c, "hullo");
    MT_CHECK_THAT(str_equals(&a, &c) == false);

    str_append_one(&b, "!");
    MT_CHECK_THAT(str_equals(&a, &b) == false);
    MT_CHECK_THAT(str_equals(NULL, &a) == false);
    MT_CHECK_THAT(str_equals(&a, NULL) == false);

    str_free(&a);
    str_free(&b);
    str_free(&c);
}

MT_DEFINE_TEST(equals_cstr)
{
    String s = str_init();
    MT_CHECK_THAT(str_equals_cstr(&s, "") == true);

    str_clear(&s);
    str_append_one(&s, "world");
    MT_CHECK_THAT(str_equals_cstr(&s, "world") == true);
    MT_CHECK_THAT(str_equals_cstr(&s, "world!") == false);
    MT_CHECK_THAT(str_equals_cstr(&s, "wurld") == false);
    MT_CHECK_THAT(str_equals_cstr(&s, NULL) == false);
    MT_CHECK_THAT(str_equals_cstr(NULL, "world") == false);

    str_free(&s);
}

MT_DEFINE_TEST(equals_n)
{
    String s = str_init();

    const char *any = "ignored";
    MT_CHECK_THAT(str_equals_n(&s, any, 0) == true);

    str_append_one(&s, "Hello");

    const char buf[] = { 'H','e','l','l','o','\0','G','A','R','B' };

    MT_CHECK_THAT(str_equals_n(&s, buf, 5) == true);
    MT_CHECK_THAT(str_equals_n(&s, buf, 6) == false);
    MT_CHECK_THAT(str_equals_n(&s, "Hello!", 6) == false);
    MT_CHECK_THAT(str_equals_n(&s, "Hellu", 5) == false);
    MT_CHECK_THAT(str_equals_n(&s, NULL, 5) == false);
    MT_CHECK_THAT(str_equals_n(NULL, buf, 5) == false);

    str_free(&s);
}

MT_DEFINE_TEST(write_and_read_file)
{
    String str = str_init();
    str_append_one(&str, "alpha\nbeta\ngamma");

    FILE *f = tmpfile();
    MT_ASSERT_THAT(f != NULL);

    MT_CHECK_THAT(str_write_file(&str, f) == true);

    rewind(f);

    String rd = str_init();
    MT_CHECK_THAT(str_read_file(&rd, f) == true);
    MT_CHECK_THAT(strcmp(rd.buffer, "alpha\nbeta\ngamma") == 0);

    fclose(f);
    str_free(&rd);
    str_free(&str);
}

MT_DEFINE_TEST(free)
{
    String str = str_init();

    str_append(&str, "Foo", "Bar", "Baz");
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_ASSERT_THAT(str.capacity > 0);
    MT_ASSERT_THAT(str.size > 0);

    str_free(&str);
    MT_CHECK_THAT(str.buffer == NULL);
    MT_CHECK_THAT(str.capacity == 0);
    MT_CHECK_THAT(str.size == 0);
}

MT_DEFINE_TEST(clear)
{
    String str = str_init();

    str_append(&str, "Foo", "Bar", "Baz");
    MT_ASSERT_THAT(str.buffer != NULL);
    MT_ASSERT_THAT(str.capacity > 0);
    MT_ASSERT_THAT(str.size > 0);

    str_clear(&str);
    MT_CHECK_THAT(str.buffer != NULL || true);
    MT_CHECK_THAT(str.size == 0);
    if (str.buffer) MT_CHECK_THAT(str.buffer[0] == '\0');

    str_free(&str);
}

MT_DEFINE_TEST(clone)
{
    String src = str_init();
    str_append(&src, "Foo", "Bar", "Baz");

    String dst = str_init();
    str_clone(&src, &dst);

    MT_ASSERT_THAT(src.buffer != NULL);
    MT_ASSERT_THAT(dst.buffer != NULL);
    MT_ASSERT_THAT(dst.size == src.size);

    MT_CHECK_THAT(strcmp(src.buffer, "FooBarBaz") == 0);
    MT_CHECK_THAT(strcmp(dst.buffer, "FooBarBaz") == 0);

    str_free(&src);
    str_free(&dst);
}

MT_DEFINE_TEST(move)
{
    String src = str_init();
    str_append(&src, "Foo", "Bar", "Baz");

    String dst = str_move(&src);

    MT_ASSERT_THAT(dst.buffer != NULL);
    MT_ASSERT_THAT(dst.size == 9);
    MT_CHECK_THAT(strcmp(dst.buffer, "FooBarBaz") == 0);

    MT_ASSERT_THAT(src.size == 0);
    if (src.buffer) {
        MT_CHECK_THAT(src.buffer[0] == '\0');
    } else {
        MT_CHECK_THAT(src.capacity == 0);
    }

    str_free(&dst);
    str_free(&src);
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
    MT_RUN_TEST(append_str);
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
    MT_RUN_TEST(equals);
    MT_RUN_TEST(equals_cstr);
    MT_RUN_TEST(equals_n);

    MT_RUN_TEST(write_and_read_file);

    MT_RUN_TEST(free);
    MT_RUN_TEST(clear);

    MT_RUN_TEST(clone);
    MT_RUN_TEST(move);

    MT_PRINT_SUMMARY();
    return MT_EXIT_CODE;
}
