#include "tc/groups.hpp"

#include <sstream>

namespace tc {
    Group schlafli(const std::vector<int> &mults) {
        Group res(mults.size() + 1);
        for (int i = 0; i < mults.size(); ++i) {
            res.set(Rel{i, i + 1, mults[i]});
        }
        return res;
    }

    namespace group {
        Group A(const int dim) {
            if (dim == 0) return Group(0);
            return schlafli(std::vector<int>(dim - 1, 3));
        }

        Group B(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[0] = 4;

            return schlafli(mults);
        }

        Group D(const int dim) {
            if (dim <= 2) {
                throw std::runtime_error("tc::group::D requires dim > 2.");
            }
            
            std::vector<int> mults(dim - 1, 3);
            mults[dim - 2] = 2;

            Group g = schlafli(mults);
            g.set(Rel{1, dim - 1, 3});

            return g;
        }

        Group E(const int dim) {
            if (dim <= 3) {
                throw std::runtime_error("tc::group::E requires dim > 3.");
            }
            
            std::vector<int> mults(dim - 1, 3);
            mults[dim - 2] = 2;

            Group g = schlafli(mults);
            g.set(Rel{2, dim - 1, 3});

            return g;
        }

        Group F4() {
            return schlafli({3, 4, 3});
        }

        Group G2() {
            return schlafli({6});
        }

        Group H(const int dim) {
            std::vector<int> mults(dim - 1, 3);
            mults[0] = 5;

            return schlafli(mults);
        }

        Group I2(const int n) {
            return schlafli({n});
        }

        Group T(const int n, const int m) {
            return schlafli({n, 2, m});
        }

        Group T(const int n) {
            return T(n, n);
        }
    }
}
