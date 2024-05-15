#include "Util.h"

double EuclideanDistance(const Coor &p1, const Coor &p2){
    return pow(p1.getX() - p2.getX(), 2) + pow(p1.getY() - p2.getY(), 2);
}

double MangattanDistance(const Coor &p1, const Coor &p2){
    return abs(p1.getX() - p2.getX()) + abs(p1.getY() - p2.getY());
}
