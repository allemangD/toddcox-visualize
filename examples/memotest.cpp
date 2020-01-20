#include <geometry.hpp>
#include <tc/groups.hpp>
#include <iostream>
#include <chrono>

int main() {
    tc::Group g = tc::group::B(3);
    GeomGen m(g);

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
    GeomGen mbig(big);

    auto s1 = std::chrono::system_clock::now();
    auto res1 = mbig.solve({0,1,2,3,4,7}, {2,4,7});
    auto e1 = std::chrono::system_clock::now();

    std::chrono::duration<double> t1 = e1 - s1;
    std::cout << t1.count() << ": " << res1.size() << std::endl;

    auto s2 = std::chrono::system_clock::now();
    auto res2 = mbig.solve({0,2,4,7,1,3}, {4,7,2});
    auto e2 = std::chrono::system_clock::now();

    std::chrono::duration<double> t2 = e2 - s2;
    std::cout << t2.count() << ": " << res2.size() << std::endl;
}
