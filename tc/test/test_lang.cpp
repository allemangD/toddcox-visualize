#include <tc/group.hpp>
#include <tc/groups.hpp>

#include <gtest/gtest.h>

// "5 3 3"
// "5 (3 3)"
// "[5 3 3]"
// "[4 3 [3 5] 3]"
// "{3 4 5 6 7 8 9}"
// "3 {3 3 [4] 3} 5"
// "5 * 3"
// "5 * [3]"
// "5 * {2 3}"
// "5 * [3 2]"
// "(5 2) * [3 2]"
// "4 [3 * [2 3]] 5"
// "{3 * 3} [4] [5]"

TEST(coxeter, simple) {
    auto g = tc::coxeter("5 3 3");

    ASSERT_EQ(g.ngens, 4);

    EXPECT_EQ(g.get(0, 1), 5);
    EXPECT_EQ(g.get(1, 2), 3);
    EXPECT_EQ(g.get(3, 4), 3);

    EXPECT_EQ(g.get(0, 2), 2);
    EXPECT_EQ(g.get(0, 3), 2);
    EXPECT_EQ(g.get(0, 4), 2);
    EXPECT_EQ(g.get(1, 3), 2);
    EXPECT_EQ(g.get(1, 4), 2);
    EXPECT_EQ(g.get(2, 4), 2);
}

TEST(coxeter, looping) {
    auto g = tc::coxeter("{5 3 4}");

    ASSERT_EQ(g.ngens, 3);

    EXPECT_EQ(g.get(0, 1), 5);
    EXPECT_EQ(g.get(1, 2), 3);
    EXPECT_EQ(g.get(2, 0), 4);
}
