#include <ctime>
#include <vector>

#include <tc/groups.hpp>
#include <tc/core.hpp>

#include <gtest/gtest.h>

/// helper for testing solve and testing speed
testing::AssertionResult AssertSolveOrder(
    const char *group_expr,
    const char *sub_gens_expr,
    const char *expected_order_expr,
    const tc::Group &group,
    const std::vector<size_t> &sub_gens,
    size_t expected_order
) {
    auto start = std::clock();
    auto cosets = group.solve(sub_gens);
    auto end = std::clock();

    size_t actual_order = cosets.order();

    auto total_sec = (double) (end - start) / CLOCKS_PER_SEC;
    auto cosets_per_sec = (double) actual_order / total_sec;

    bool order_good = actual_order == expected_order;
    bool speed_good = cosets_per_sec >= MIN_COS_PER_SEC || total_sec < 0.0001;
    // extremely short times cause false negatives. ex. A2 can be solved in only 3 clocks.

    if (order_good && speed_good) {
        return testing::AssertionSuccess();
    }

    testing::AssertionResult res = testing::AssertionFailure();
    res << group_expr << " / " << sub_gens_expr << " :";
    if (!order_good) {
        res << " Gave order " << actual_order << " but expected order " << expected_order << ".";
    }
    if (!speed_good) {
        res << " Solution too slow (" << cosets_per_sec
            << " cos/s < " << MIN_COS_PER_SEC << ")."
            << " " << std::fixed << total_sec << " s.";
    }

    return res;
}

#define EXPECT_SOLVE_ORDER(group, sub_gens, expected_order) EXPECT_PRED_FORMAT3(AssertSolveOrder, group, sub_gens, expected_order);

using v = std::vector<size_t>;

tc::Group A(unsigned int n) {
    return tc::vcoxeter("3 * {}", {n - 1});
}

tc::Group B(unsigned int n) {
    return tc::vcoxeter("4 3 * {}", {n - 2});
}

tc::Group D(unsigned int n) {
    return tc::vcoxeter("3 * [1 1 {}]", {n - 3});
}

tc::Group E(unsigned int n) {
    return tc::vcoxeter("3 * [1 2 {}]", {n - 4});
}

tc::Group F4() {
    return tc::coxeter("3 4 3");
}

tc::Group G2() {
    return tc::coxeter("6");
}

tc::Group H(unsigned int n) {
    return tc::vcoxeter("5 3 * {}", {n - 2});
}

tc::Group I2(unsigned int n) {
    return tc::vcoxeter("{}", {n});
}

tc::Group T(unsigned int m, unsigned int n) {
    return tc::vcoxeter("{} 2 {}", {m, n});
}

tc::Group T(unsigned int n) {
    return T(n, n);
}

// See the group orders here https://en.wikipedia.org/wiki/Coxeter_group#Properties

TEST(solve, A) {
    EXPECT_SOLVE_ORDER(A(1), v({}), 2);
    EXPECT_SOLVE_ORDER(A(2), v({}), 6);
    EXPECT_SOLVE_ORDER(A(3), v({}), 24);
    EXPECT_SOLVE_ORDER(A(3), v({0}), 12);
    EXPECT_SOLVE_ORDER(A(3), v({0, 1}), 4);
    EXPECT_SOLVE_ORDER(A(3), v({0, 2}), 6);
    EXPECT_SOLVE_ORDER(A(3), v({2}), 12);
    EXPECT_SOLVE_ORDER(A(4), v({}), 120);
    EXPECT_SOLVE_ORDER(A(4), v({0}), 60);
    EXPECT_SOLVE_ORDER(A(4), v({0, 1}), 20);
    EXPECT_SOLVE_ORDER(A(4), v({2}), 60);
    EXPECT_SOLVE_ORDER(A(4), v({0, 2}), 30);
}

TEST(solve, B) {
    EXPECT_SOLVE_ORDER(B(2), v({}), 8);
    EXPECT_SOLVE_ORDER(B(3), v({}), 48);
    EXPECT_SOLVE_ORDER(B(3), v({0}), 24);
    EXPECT_SOLVE_ORDER(B(3), v({0, 2}), 12);
    EXPECT_SOLVE_ORDER(B(4), v({}), 384);
    EXPECT_SOLVE_ORDER(B(4), v({0}), 192);
    EXPECT_SOLVE_ORDER(B(4), v({0, 2}), 96);
    EXPECT_SOLVE_ORDER(B(5), v({}), 3840);
    EXPECT_SOLVE_ORDER(B(5), v({0}), 1920);
    EXPECT_SOLVE_ORDER(B(5), v({0, 2}), 960);
    EXPECT_SOLVE_ORDER(B(5), v({0, 2, 3}), 320);
    EXPECT_SOLVE_ORDER(B(6), v({}), 46080);
    EXPECT_SOLVE_ORDER(B(6), v({0}), 23040);
    EXPECT_SOLVE_ORDER(B(6), v({0, 2}), 11520);
    EXPECT_SOLVE_ORDER(B(6), v({0, 2, 3}), 3840);
    EXPECT_SOLVE_ORDER(B(6), v({0, 2, 3, 5}), 1920);
}

TEST(solve, D) {
    EXPECT_SOLVE_ORDER(D(3), v({}), 24);
    EXPECT_SOLVE_ORDER(D(4), v({}), 192);
    EXPECT_SOLVE_ORDER(D(4), v({0, 1}), 32);
    EXPECT_SOLVE_ORDER(D(4), v({0, 1, 3}), 8);
    EXPECT_SOLVE_ORDER(D(5), v({}), 1920);
    EXPECT_SOLVE_ORDER(D(5), v({0, 1}), 320);
    EXPECT_SOLVE_ORDER(D(5), v({0, 1, 3}), 80);
    EXPECT_SOLVE_ORDER(D(5), v({0, 1, 3, 4}), 16);
    EXPECT_SOLVE_ORDER(D(6), v({}), 23040);
    EXPECT_SOLVE_ORDER(D(6), v({0, 1}), 3840);
    EXPECT_SOLVE_ORDER(D(6), v({0, 1, 3}), 960);
    EXPECT_SOLVE_ORDER(D(6), v({0, 1, 3, 5}), 480);
}

TEST(solve, E) {
    EXPECT_SOLVE_ORDER(E(4), v({}), 120);
    EXPECT_SOLVE_ORDER(E(4), v({2}), 60);
    EXPECT_SOLVE_ORDER(E(4), v({2, 1}), 30);
    EXPECT_SOLVE_ORDER(E(4), v({2, 1, 3}), 10);
    EXPECT_SOLVE_ORDER(E(5), v({}), 1920);
    EXPECT_SOLVE_ORDER(E(5), v({2}), 960);
    EXPECT_SOLVE_ORDER(E(5), v({2, 1}), 480);
    EXPECT_SOLVE_ORDER(E(5), v({2, 1, 3}), 160);
    EXPECT_SOLVE_ORDER(E(6), v({}), 51840);
    EXPECT_SOLVE_ORDER(E(6), v({2}), 25920);
    EXPECT_SOLVE_ORDER(E(6), v({2, 1}), 12960);
    EXPECT_SOLVE_ORDER(E(6), v({2, 1, 3}), 4320);
}

TEST(solve, F) {
    EXPECT_SOLVE_ORDER(F4(), v({}), 1152);
    EXPECT_SOLVE_ORDER(F4(), v({0}), 576);
    EXPECT_SOLVE_ORDER(F4(), v({0, 2}), 288);
    EXPECT_SOLVE_ORDER(F4(), v({1, 3}), 288);
    EXPECT_SOLVE_ORDER(F4(), v({1, 2, 3}), 24);
}

TEST(solve, G) {
    EXPECT_SOLVE_ORDER(G2(), v({}), 12);
    EXPECT_SOLVE_ORDER(G2(), v({0}), 6);
    EXPECT_SOLVE_ORDER(G2(), v({1}), 6);
}

TEST(solve, H) {
    EXPECT_SOLVE_ORDER(H(2), v({}), 10);
    EXPECT_SOLVE_ORDER(H(2), v({0}), 5);
    EXPECT_SOLVE_ORDER(H(2), v({1}), 5);
    EXPECT_SOLVE_ORDER(H(3), v({}), 120);
    EXPECT_SOLVE_ORDER(H(3), v({0}), 60);
    EXPECT_SOLVE_ORDER(H(3), v({0, 1}), 12);
    EXPECT_SOLVE_ORDER(H(3), v({0, 2}), 30);
    EXPECT_SOLVE_ORDER(H(3), v({1, 2}), 20);
    EXPECT_SOLVE_ORDER(H(4), v({}), 14400);
    EXPECT_SOLVE_ORDER(H(4), v({0}), 7200);
    EXPECT_SOLVE_ORDER(H(4), v({1}), 7200);
    EXPECT_SOLVE_ORDER(H(4), v({1, 3}), 3600);
}

TEST(solve, I) {
    EXPECT_SOLVE_ORDER(I2(2), v({}), 4);
    EXPECT_SOLVE_ORDER(I2(3), v({}), 6);
    EXPECT_SOLVE_ORDER(I2(3), v({0}), 3);
    EXPECT_SOLVE_ORDER(I2(3), v({1}), 3);
    EXPECT_SOLVE_ORDER(I2(4), v({}), 8);
    EXPECT_SOLVE_ORDER(I2(4), v({0}), 4);
    EXPECT_SOLVE_ORDER(I2(4), v({1}), 4);
    EXPECT_SOLVE_ORDER(I2(5), v({}), 10);
    EXPECT_SOLVE_ORDER(I2(5), v({0}), 5);
    EXPECT_SOLVE_ORDER(I2(5), v({1}), 5);
}

TEST(solve, T) {
    EXPECT_SOLVE_ORDER(T(3), v({}), 36);
    EXPECT_SOLVE_ORDER(T(4), v({}), 64);
    EXPECT_SOLVE_ORDER(T(400), v({}), 640000);
    EXPECT_SOLVE_ORDER(T(400), v({0}), 320000);
    EXPECT_SOLVE_ORDER(T(400), v({0, 2}), 160000);
    EXPECT_SOLVE_ORDER(T(400, 300), v({}), 480000);
    EXPECT_SOLVE_ORDER(T(400, 300), v({0}), 240000);
    EXPECT_SOLVE_ORDER(T(400, 300), v({0, 2}), 120000);
}

TEST(solve_large, B) {
    EXPECT_SOLVE_ORDER(B(7), v({}), 645120);
    EXPECT_SOLVE_ORDER(B(8), v({}), 10321920);
}

TEST(solve_large, E) {
    EXPECT_SOLVE_ORDER(E(6), v({}), 51840);
    EXPECT_SOLVE_ORDER(E(7), v({}), 2903040);
}

TEST(solve_large, T) {
    EXPECT_SOLVE_ORDER(T(500), v({}), 1000000);
    EXPECT_SOLVE_ORDER(T(1000), v({}), 4000000);
}
