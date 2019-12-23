#include "solver.cpp"

Group A(const int n) {
    if (n == 0)
        return Group(0);

    return Group::schlafli(std::vector<int>(n-1,3));
}

Group B(const int n) {
    std::vector<int> mults(n-1,3);
    mults[0] = 4;
    return Group::schlafli(mults);
}

Group D(const int n) {
    std::vector<int> mults(n-1,3);
    mults[n-2] = 2;
    Group g = Group::schlafli(mults);
    g.setmult({1,n-1,3});
    return g;
}

Group E(const int n) {
    std::vector<int> mults(n-1,3);
    mults[n-2] = 2;
    Group g = Group::schlafli(mults);
    g.setmult({2,n-1,3});
    return g;
}

Group F4() {
    return Group::schlafli({3,4,3});
}

Group G2() {
    return Group::schlafli({6});
}

Group H(const int n) {
    std::vector<int> mults(n-1,3);
    mults[0] = 5;
    return Group::schlafli(mults);
}

Group I2(const int n) {
    return Group::schlafli({n});
}

Group T(const int n) {
    return I2(n)^2;
}
