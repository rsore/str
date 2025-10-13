#define SBDEF static inline
#define SB_IGNORE_NODISCARD
#define SB_IMPLEMENTATION
#include "../sb.h"

#include "minitest.h"

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

MT_DEFINE_TEST(reset)
{
    StringBuilder sb = sb_init();

    sb_append(&sb, "Foo", "Bar", "Baz");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity > 0);
    MT_ASSERT_THAT(sb.size > 0);

    sb_reset(&sb);
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

    MT_RUN_TEST(strdup);
    MT_RUN_TEST(release);
    MT_RUN_TEST(shrink_and_release);

    MT_RUN_TEST(free);
    MT_RUN_TEST(reset);

    MT_RUN_TEST(clone);
    MT_RUN_TEST(move);

    MT_PRINT_SUMMARY();
    return MT_EXIT_CODE;
}
