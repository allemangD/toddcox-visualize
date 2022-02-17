#include <Eigen/Eigen>

#include <fstream>
#include <iostream>

#include <ml/meshlib.hpp>
#include <ml/meshlib_json.hpp>

auto make_circle(float radius, size_t npoints = 32) {
    Eigen::Array<float, 1, Eigen::Dynamic> theta(1, npoints);
    for (int i = 0; i < theta.size(); ++i) {
        theta(i) = (float) M_PI * 2.0f * (float) i / (float) npoints;
    }

    Eigen::Array<float, 3, Eigen::Dynamic> points(3, npoints);
    points.row(0) = theta.sin();
    points.row(1) = theta.cos();
    points.row(2).setZero();

    Eigen::Array<unsigned int, 1, Eigen::Dynamic> idx(1, npoints - 1);
    for (int i = 0; i < idx.size(); ++i) {
        idx(i) = i;
    }

    Eigen::Array<unsigned int, 3, Eigen::Dynamic> cells(3, npoints - 1);
    cells.row(0) = 0;
    cells.row(1) = idx;
    cells.row(2) = idx + 1;

    return ml::Mesh(points, cells);
}

int main() {
    std::string path = "circle.pak";

    auto omesh = make_circle(1.0f);
    using MT = decltype(omesh);

    ml::write(omesh, std::ofstream(path, std::ios::out | std::ios::binary));

    auto imesh = ml::read<MT>(std::ifstream(path, std::ios::in | std::ios::binary));

    std::cout << imesh.points << std::endl;
    std::cout << imesh.cells << std::endl;
}
