#include <tc/groups.hpp>

#include <fmt/args.h>
#include <fmt/core.h>

namespace tc {
    Group schlafli(const std::vector<unsigned int> &mults) {
        Group res(mults.size() + 1);
        for (size_t i = 0; i < mults.size(); ++i) {
            res.set(Rel{i, i + 1, mults[i]});
        }
        return res;
    }

    Group vcoxeter(const std::string &symbol, const std::vector<unsigned int> &values) {
        fmt::dynamic_format_arg_store<fmt::format_context> ds;

        for (const auto &value: values) {
            ds.push_back(value);
        }

        return coxeter(fmt::vformat(symbol, ds));
    }
}
