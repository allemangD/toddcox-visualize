//
// Created by raymo on 1/20/2020.
//

#include <tc/groups.hpp>
#include <geometry.hpp>
#include <iostream>

int main() {
    auto g = tc::schlafli({3, 2});
    GeomGen gg(g);

    auto path = gg.solve().path;

    //std::vector<std::string>  = {"a", "b", "c"};
    std::string base = "";
    auto words = path.walk<std::string, std::string>(base, {"a", "b", "c"}, [](auto s1, auto g) { return s1 + g; });
    for (const auto word : words) {
        std::cout << word << std::endl;
    }

    std::vector<int> gens = {0, 1, 2};
    auto s = gg.triangulate(gens);
    s.print();
    return 0;

    auto g_gens = gg.group_gens();
    std::vector<int> sg_gens = {1, 2};
    auto ns = gg.tile(g_gens, sg_gens, s);

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