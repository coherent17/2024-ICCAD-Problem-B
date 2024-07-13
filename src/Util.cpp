#include "Util.h"

Param::Param():
    MAX_SQUARE_DISPLACEMENT(400000000),
    MAX_BANDWIDTH(10000)
    {}
Param::~Param(){}

double SquareEuclideanDistance(const Coor &p1, const Coor &p2){
    return std::pow(p1.getX() - p2.getX(), 2) + std::pow(p1.getY() - p2.getY(), 2);
}


double MangattanDistance(const Coor &p1, const Coor &p2){
    return std::abs(p1.getX() - p2.getX()) + std::abs(p1.getY() - p2.getY());
}

double GaussianKernel(const Coor &p1, const Coor &p2, double bandwidth){
    double numerator = SquareEuclideanDistance(p1, p2);
    double denominator = -2 * std::pow(bandwidth, 2);
    return std::exp(numerator / denominator);

}

double HPWL(const Coor& c1, const Coor& c2){
    return std::abs(c1.x - c2.x) + std::abs(c1.y - c2.y);
}