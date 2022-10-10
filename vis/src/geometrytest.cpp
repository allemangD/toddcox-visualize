#include <tc/groups.hpp>
#include <geo/geometry.hpp>
#include <geo/solver.hpp>
#include <geo/mirror.hpp>

#include <ml/meshlib.hpp>
#include <ml/meshlib_json.hpp>

#include <fstream>
#include <iostream>

template<int N>
using ProjectiveNf = Eigen::Transform<float, N, Eigen::Projective>;

template<int N>
using VectorNf = Eigen::Vector<float, N>;

template<int N>
Eigen::Matrix<float, N, Eigen::Dynamic> make_points(
    const tc::Group &group,
    const Eigen::Vector<float, N> &root
) {
    // todo clean up mirror / plane_intersections / barycentric
    // ideally barycentric will work in rotors, so that stellations etc. will be possible
    auto mirrors = mirror<N>(group);
    auto corners = plane_intersections(mirrors);
    auto start = barycentric(corners, root);

    auto cosets = group.solve();

    auto verts = cosets.path.walk<VectorNf<N>, VectorNf<N>>(start, mirrors, reflect<vec4>);

    Eigen::Matrix<float, N, Eigen::Dynamic> points(root.size(), verts.size());
    std::copy(verts.begin(), verts.end(), points.colwise().begin());

    return points;
}

template<int N>
Eigen::Matrix<unsigned, N, Eigen::Dynamic> make_cells(
    const tc::Group &group,
    const std::vector<std::vector<int>> &exclude = {}
) {
    auto gens = generators(group);
    auto combos = combinations(gens, N - 1);

    // todo clean up merge(hull(...))
    Eigen::Matrix<unsigned, N, Eigen::Dynamic> cells = merge<N>(hull<N>(group, combos, exclude));

    return cells;
}

int main() {
    std::vector<int> symbol = {5, 3, 3};
    auto group = tc::schlafli(symbol);

    vec4 root{0.80, 0.02, 0.02, 0.02};

    auto points = make_points(group, root);
    auto cells = make_cells<3>(group, {{0, 1}});

    ml::Mesh mesh(points, cells);

//    auto transform = ProjectiveNf<vec4::RowsAtCompileTime>::Identity();
//    transform.translation() << 0, 0.5, 0, 0;
//    points = (transform * points.colwise().homogeneous()).colwise().hnormalized();

    std::cout << points.rows() << " " << points.cols() << std::endl;
    std::cout << cells.rows() << " " << cells.cols() << std::endl;

    ml::write(mesh, std::ofstream("dodeca.pak", std::ios::out | std::ios::binary));

    return EXIT_SUCCESS;
}
