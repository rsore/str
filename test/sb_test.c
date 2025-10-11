#define SBDEF static inline
#define SB_IGNORE_NODISCARD
#define SB_IMPLEMENTATION
#include "../sb.h"

#include "minitest.h"

MT_DEFINE_TEST(init)
{
    StringBuilder sb = sb_init();
    MT_CHECK_THAT(sb.buffer == NULL);
    MT_CHECK_THAT(sb.capacity == 0);
    MT_CHECK_THAT(sb.size == 0);
}

MT_DEFINE_TEST(reserve)
{
    StringBuilder sb = sb_init();
    MT_ASSERT_THAT(sb.buffer == NULL);
    MT_ASSERT_THAT(sb.capacity == 0);

    sb_reserve(&sb, 16);
    MT_CHECK_THAT(sb.buffer != NULL);
    MT_CHECK_THAT(sb.capacity >= 16);
}

MT_DEFINE_TEST(append_one)
{
    StringBuilder sb = sb_init();
    MT_ASSERT_THAT(sb.buffer == NULL);
    MT_ASSERT_THAT(sb.capacity == 0);
    MT_ASSERT_THAT(sb.size == 0);

    sb_append_one(&sb, "Hello");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 5);
    MT_ASSERT_THAT(sb.size == 5);

    MT_CHECK_THAT(memcmp(sb.buffer, "Hello", 5) == 0);

    sb_append_one(&sb, " world");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 11);
    MT_ASSERT_THAT(sb.size == 11);

    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world", 11) == 0);
}

MT_DEFINE_TEST(append)
{
    StringBuilder sb = sb_init();
    MT_ASSERT_THAT(sb.buffer == NULL);
    MT_ASSERT_THAT(sb.capacity == 0);
    MT_ASSERT_THAT(sb.size == 0);

    sb_append(&sb, "Hello", " world");
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 11);
    MT_ASSERT_THAT(sb.size == 11);

    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world", 11) == 0);
}

MT_DEFINE_TEST(append_char)
{
    StringBuilder sb = sb_init();
    MT_ASSERT_THAT(sb.buffer == NULL);
    MT_ASSERT_THAT(sb.capacity == 0);
    MT_ASSERT_THAT(sb.size == 0);

    sb_append_char(&sb, 'a');
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 1);
    MT_ASSERT_THAT(sb.size == 1);
    MT_CHECK_THAT(sb.buffer[0] == 'a');

    sb_append_char(&sb, 'b');
    MT_ASSERT_THAT(sb.buffer != NULL);
    MT_ASSERT_THAT(sb.capacity >= 2);
    MT_ASSERT_THAT(sb.size == 2);
    MT_CHECK_THAT(sb.buffer[0] == 'a');
    MT_CHECK_THAT(sb.buffer[1] == 'b');

}

MT_DEFINE_TEST(append_sb)
{
    StringBuilder sb1 = sb_init();
    StringBuilder sb2 = sb_init();

    sb_append(&sb1, "Hello", " world");
    sb_append(&sb2, "Foo", " bar", " ", "baz");

    sb_append_sb(&sb1, &sb2);
    MT_CHECK_THAT(memcmp(sb1.buffer, "Hello worldFoo bar baz", sb1.size) == 0);
}

MT_DEFINE_TEST(appendf)
{
    StringBuilder sb = sb_init();

    sb_appendf(&sb, "Hello %s", "world");
    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world", sb.size) == 0);

    sb_appendf(&sb, " %d + %d = %d", 34, 35, 69);
    MT_CHECK_THAT(memcmp(sb.buffer, "Hello world 34 + 35 = 69", sb.size) == 0);
}

MT_DEFINE_TEST(to_cstr)
{
    StringBuilder sb = sb_init();
    char *cstr = sb_to_cstr(&sb);
    MT_ASSERT_THAT(cstr != NULL); // Empty builder should return empty string, not NULL
    MT_CHECK_THAT(strcmp(cstr, "") == 0);

    sb_append(&sb, "Hello", "world");
    cstr = sb_to_cstr(&sb);
    MT_CHECK_THAT(strcmp(cstr, "Helloworld") == 0);
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
    MT_CHECK_THAT(sb.buffer != NULL);
    MT_CHECK_THAT(sb.capacity > 0);
    MT_CHECK_THAT(sb.size == 0);
}

MT_DEFINE_TEST(clone)
{
    StringBuilder src = sb_init();
    sb_append(&src, "Foo", "Bar", "Baz");

    StringBuilder dst = sb_init();
    sb_clone(&src, &dst);

    MT_ASSERT_THAT(src.buffer != NULL);
    MT_ASSERT_THAT(src.size == src.size);

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

    MT_ASSERT_THAT(src.buffer   == NULL);
    MT_ASSERT_THAT(src.size     == 0);
    MT_ASSERT_THAT(src.capacity == 0);

    MT_ASSERT_THAT(dst.buffer != NULL);
    MT_ASSERT_THAT(dst.size == 9);

    MT_CHECK_THAT(strcmp(dst.buffer, "FooBarBaz") == 0);
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

    MT_RUN_TEST(to_cstr);

    MT_RUN_TEST(free);
    MT_RUN_TEST(reset);

    MT_RUN_TEST(clone);
    MT_RUN_TEST(move);

    MT_PRINT_SUMMARY();
    return MT_EXIT_CODE;
}
