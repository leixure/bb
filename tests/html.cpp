// Test html.h
#include <iostream>

#include <gtest/gtest.h>

#include "../src/html.h"

using namespace std;
using namespace bb;

namespace {
    TEST(HTMLTest, echo_single) {
        ostringstream out;
        out << html::echo("hello");
        EXPECT_EQ(out.str(), "hello");
    }
    TEST(HTMLTest, echo_multiple) {
        ostringstream out;
        out << html::echo("hello, ", "world!");
        EXPECT_EQ(out.str(), "hello, world!");
    }
}
