#include <iostream>
#include <yaml-cpp/yaml.h>
#include <string>

int main() {
    auto cfg = YAML::LoadFile("presets/default.yaml");

    for (const auto &group : cfg["groups"]) {
//        std::cout << group["symbol"] << std::endl;
        auto s = group["symbol"].as<std::vector<int>>();
        for (const auto e : s) std::cout << e << " ";
        std::cout << std::endl;
    }
}
