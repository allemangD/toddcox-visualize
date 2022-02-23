#include <tc/core.hpp>
#include <tc/groups.hpp>
#include <Eigen/Eigen>
#include <iostream>

int main() {
    auto group = tc::group::H(4);
    auto cos = group.solve({});

    std::cout << cos.size() << std::endl;
}
