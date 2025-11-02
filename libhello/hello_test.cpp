#include "libhello/hello.h"

#include "gtest/gtest.h"

TEST(TestHello, HelloUser)
{
    ASSERT_EQ("Hello, user!", libhello::hello("user"));
}
