#include <geometry.hpp>
#include <tc/groups.hpp>
#include <solver.hpp>
#include <mirror.hpp>

#include <ml/meshlib.hpp>
#include <ml/meshlib_json.hpp>

#include <fstream>
#include <iostream>

int main() {
    std::vector<int> symbol = {5, 3, 3};
    vec4 root{0.80, 0.02, 0.02, 0.02};


    auto group = tc::schlafli(symbol);
    auto gens = generators(group);
    auto combos = combinations(gens, 2);

    const auto &inds = merge<3>(hull<3>(group, combos, {
        {0, 1},
    }));

    auto cosets = group.solve();
    auto mirrors = mirror<4>(group);
    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, root);

    using Projective = Eigen::Transform<float, 4, Eigen::Projective>;
    Projective transform = Projective::Identity();

    auto higher = cosets.path.walk<vec4, vec4>(start, mirrors, reflect<vec4>);
    std::transform(
        higher.begin(), higher.end(), higher.begin(),
        [&](const vec4 &v) {
            return (transform * v.homogeneous()).hnormalized();
        }
    );

//    std::vector<vec4> lower(higher.size());
//    std::transform(higher.begin(), higher.end(), lower.begin(), stereo<4>);
//    const auto &verts = lower;

    const auto &verts = higher;

    using PT = Eigen::Matrix<float, 4, Eigen::Dynamic>;
    using CT = Eigen::Matrix<unsigned, 3, Eigen::Dynamic>;

    PT points(4, verts.size());
    std::copy(verts.begin(), verts.end(), points.colwise().begin());

    CT cells = inds;

    std::cout << points.rows() << " " << points.cols() << std::endl;
    std::cout << cells.rows() << " " << cells.cols() << std::endl;

    ml::Mesh mesh(points, cells);
    ml::write(mesh, std::ofstream("dodeca.pak", std::ios::out | std::ios::binary));

    return EXIT_SUCCESS;
}
