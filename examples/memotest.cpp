#include <geometry.hpp>
#include <tc/groups.hpp>
#include <iostream>
#include <ctime>

int main() {
    tc::Group g = tc::group::B(3);
    CosetMemo m(g);

    m.solve({}, {});
    m.solve({0}, {});
    m.solve({0}, {0});
    m.solve({1}, {});
    m.solve({1}, {1});
    m.solve({2}, {});
    m.solve({2}, {2});
    m.solve({0, 1}, {});
    m.solve({0, 1}, {0});
    m.solve({0, 1}, {1});
    m.solve({0, 1}, {0, 1});
    m.solve({1, 2}, {});
    m.solve({1, 2}, {1});
    m.solve({1, 2}, {2});
    m.solve({1, 2}, {1, 2});
    m.solve({0, 2}, {});
    m.solve({0, 2}, {0});
    m.solve({0, 2}, {2});
    m.solve({0, 2}, {0, 2});
    m.solve({0, 1, 2}, {});
    m.solve({0, 1, 2}, {0});
    m.solve({0, 1, 2}, {1});
    m.solve({0, 1, 2}, {2});
    m.solve({0, 1, 2}, {0, 1});
    m.solve({0, 1, 2}, {1, 2});
    m.solve({0, 1, 2}, {0, 2});
    m.solve({0, 1, 2}, {0, 1, 2});

    tc::Group big = tc::group::B(8);
    CosetMemo mbig(big);

    auto s1 = clock();
    m.solve({0, 1, 2, 3, 4, 5, 6, 7}, {});
    auto e1 = clock();

    double t1 = (double) (e1 - s1) / (double) CLOCKS_PER_SEC;
    std::cout << t1 << std::endl;

    auto s2 = clock();
    m.solve({0, 1, 2, 3, 4, 5, 6, 7}, {});
    auto e2 = clock();

    double t2 = (double) (e2 - s2) / (double) CLOCKS_PER_SEC;
    std::cout << t2 << std::endl;
}
