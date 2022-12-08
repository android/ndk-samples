#include "adder.h"

#include "gtest/gtest.h"

TEST(adder, adder) { EXPECT_EQ(3, add(1, 2)); }
