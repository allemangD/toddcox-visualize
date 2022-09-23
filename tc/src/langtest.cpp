#include <iostream>

#include <tc/group.hpp>
#include <tc/groups.hpp>

int main() {
    std::vector<std::string> symbols = {
        "5 3 3",
        "5 (3 3)",
        "[5 3 3]",
        "[4 3 [3 5] 3]",
        "{3 4 5 6 7 8 9}",
        "3 {3 3 [4] 3} 5",
        "5 * 3",
        "5 * [3]",
        "5 * {2 3}",
        "5 * [3 2]",
        "(5 2) * [3 2]",
        "4 [3 * [2 3]] 5",
        "{3 * 3} [4] [5]",
    };

    for (const auto &symbol: symbols) {
        auto group = tc::coxeter(symbol);
        
        std::cout << "'" << symbol << "'" << std::endl;

        for (int i = 0; i < group.ngens; ++i) {
            for (int j = 0; j < group.ngens; ++j) {
                std::cout << group.get(i, j) << " ";
            }
            std::cout << std::endl;
        }
        
        std::cout << "===========================" << std::endl;
    }
    
    return EXIT_SUCCESS;
}
