#include <cassert>

#include "tc/groups.hpp"

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
            assert(dim >= 1);
            
            return schlafli(std::vector<int>(dim - 1, 3));
        }

        Group B(const int dim) {
            assert(dim >= 2);
            
            std::vector<int> mults(dim - 1, 3);
            mults[0] = 4;

            return schlafli(mults);
        }

        Group D(const int dim) {
            assert(dim >= 4);
            
            std::vector<int> mults(dim - 1, 3);
            mults[dim - 2] = 2;

            Group g = schlafli(mults);
            g.set(Rel{1, dim - 1, 3});

            return g;
        }

        Group E(const int dim) {
            assert(dim >= 6);
            
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
            assert(dim >= 2);
            assert(dim <= 4);
            
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
