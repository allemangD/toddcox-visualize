#pragma once

#include <array>
#include <functional>
#include <vector>
#include <string>

#include "util.hpp"
#include "cosets.hpp"
#include "group.hpp"

namespace tc {
    using Coset = unsigned int;
    
    Cosets solve(const Group &group, const std::vector<Coset> &sub_gens);
}
