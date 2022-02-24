#include <geometry.hpp>
#include <tc/groups.hpp>
#include <solver.hpp>
#include <mirror.hpp>

#include <ml/meshlib.hpp>

#include <iostream>

int main() {
    std::vector<int> symbol = {3, 4, 3, 2};
    vec5 root{0.80, 0.02, 0.02, 0.02, 0.02};

    auto group = tc::schlafli(symbol);
    auto gens = generators(group);
    auto combos = combinations(gens, 3);

    const auto &inds = merge<4>(hull<4>(group, combos, {}));

    auto cosets = group.solve();
    auto mirrors = mirror<5>(group);
    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, root);

    using Projective5f = Eigen::Transform<float, 5, Eigen::Projective>;
    Projective5f transform = Projective5f::Identity();

    auto higher = cosets.path.walk<vec5, vec5>(start, mirrors, reflect<vec5>);
    std::transform(
        higher.begin(), higher.end(), higher.begin(),
        [&](const vec5 &v) {
            return (transform * v.homogeneous()).hnormalized();
        }
    );

    std::vector<vec4> lower(higher.size());
    std::transform(higher.begin(), higher.end(), lower.begin(), stereo<4>);

    const auto &verts = lower;

    std::cout << inds.rows() << " " << inds.cols() << std::endl;

    return EXIT_SUCCESS;
}
