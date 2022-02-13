#include <Eigen/Eigen>

#include <fstream>
#include <iostream>

#include <ml/meshlib.hpp>
#include <ml/meshlib_json.hpp>

class Circle : public ml::MeshBase {
public:
    PointsType::Scalar radius;

    Circle(PointsType::Scalar radius) : radius(radius) {}

    PointsType points() const override {
        PointsType t(1, 32);
        for (int i = 0; i < t.size(); ++i) {
            t(i) = 6.28318f * (float) i / 32;
        }

        PointsType out(3, 32);
        out.array().row(0) = t.array().sin();
        out.array().row(1) = t.array().cos();
        out.array().row(2).setZero();
        return out;
    }

    CellsType cells() const override {
        CellsType t(1, 31);
        for (int i = 0; i < t.size(); ++i) {
            t(i) = i;
        }

        CellsType out(3, 31);
        out.array().row(0) = 0;
        out.array().row(1) = t.array();
        out.array().row(2) = t.array() + 1;
        return out;
    }
};

int main() {
    auto omesh = Circle(1.0f);

    ml::write(omesh, "circle.pak");

    auto imesh = ml::read("circle.pak");

    std::cout << "= points ===============" << std::endl;
    std::cout << imesh.points() << std::endl;
    std::cout << "= cells ================" << std::endl;
    std::cout << imesh.cells() << std::endl;
}
