#pragma once

#include <array>
#include <functional>
#include <vector>
#include <string>

#include "util.hpp"
#include "cosets.hpp"
#include "group.hpp"

namespace tc {
    Cosets solve(const Group &group, const std::vector<Gen> &sub_gens, const Coset &bound = UNBOUNDED);
}
