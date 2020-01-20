//
// Created by raymo on 1/20/2020.
//

#include <tc/groups.hpp>
#include <geometry.hpp>
#include <iostream>

int main () {
    auto g = tc::group::B(3);
    GeomGen gg(g);

    Simplexes s(1);
    s.vals.push_back(0);
    s.vals.push_back(1);
    s.vals.push_back(0);
    s.vals.push_back(2);
    s.vals.push_back(1);
    s.vals.push_back(2);

    auto g_gens = gg.group_gens();
    std::vector<int> sg_gens = {1,2};
    auto ns = gg.tile(g_gens,sg_gens,s);

    std::cout << "Before: " << std::endl;
    std::cout << '\t';
    for (int val : s.vals) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    std::cout << " After: " << std::endl;
    std::cout << '\t';
    for (int val : ns.vals) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    return 0;
}