#include <functional>
#include <iostream>
#include <unordered_map>

#include <tc/pair_map.hpp>

#include <gtest/gtest.h>

/// helper for comparing against two options
template<typename T, typename U, typename V>
testing::AssertionResult AssertEqEither(
    const char *val_expr,
    const char *o1_expr,
    const char *o2_expr,
    T val, U o1, V o2
) {
    if ((val == o1) || (val == o2)) {
        return testing::AssertionSuccess();
    }

    return testing::AssertionFailure()
        << val_expr << " (" << val << ") " << "does not equal " << o1_expr
        << " (" << o1 << ") " << "or " << o2_expr << " (" << o2 << ")";
}

#define EXPECT_EQ_EITHER(val, o1, o2) EXPECT_PRED_FORMAT3(AssertEqEither, val, o1, o2)


/// Naive symmetric pair hash
size_t key(size_t i, size_t j) {
    return ((i + j) << 12) ^ i ^ j;
}

/// factory to build a simple pair_map
tc::pair_map<size_t> populate(size_t size) {
    tc::pair_map<size_t> pm(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            pm(i, j) = key(i, j);
        }
    }

    return pm;
}

TEST(pair_map, fill) {
    tc::pair_map<size_t> pm(6, 42);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            EXPECT_EQ(pm(i, j), 42);
        }
    }
}

TEST(pair_map, symmetry) {
    auto pm = populate(6);

    for (int i = 0; i < pm.size(); ++i) {
        for (int j = i; j < pm.size(); ++j) {
            EXPECT_EQ(pm(i, j), key(i, j));
            EXPECT_EQ(pm(j, i), pm(i, j));
        }
    }
}

TEST(pair_map, copy) {
    auto pm = populate(6);
    auto pm_ = pm;

    ASSERT_EQ(pm_.size(), 6);

    for (int i = 0; i < pm_.size(); ++i) {
        for (int j = i; j < pm_.size(); ++j) {
            EXPECT_EQ(pm_(i, j), pm(i, j));
            EXPECT_EQ(pm_(i, j), key(i, j));
        }
    }
}

TEST(pair_map, move) {
    auto pm = populate(6);
    auto pm_ = std::move(pm);

    ASSERT_EQ(pm_.size(), 6);

    for (int i = 0; i < pm_.size(); ++i) {
        for (int j = i; j < pm_.size(); ++j) {
            EXPECT_EQ(pm_(i, j), key(i, j));
        }
    }
}

TEST(pair_map, iterate) {
    auto pm = populate(6);

    size_t count = 0;
    for (const auto &[i, j, m]: pm) {
        EXPECT_EQ(m, key(i, j));
        count++;
    }
    EXPECT_EQ(count, 21);
}

TEST(pair_map, iterate_ref) {
    auto pm = populate(6);

    for (const auto &[i, j, m]: pm) {
        m = 42;
    }

    for (const auto &[i, j, m]: pm) {
        EXPECT_EQ(m, 42);
    }
}

TEST(pair_map, view) {
    auto pm = populate(6);

    size_t count = 0;
    for (const auto &[i, j, m]: pm.of(4)) {
        EXPECT_EQ_EITHER(4, i, j);
        count++;
    }
    EXPECT_EQ(count, pm.size());
}
