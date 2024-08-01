#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef ENABLE_DEBUG_CHECKER
#define DEBUG_MSG(message) std::cout << "[DEBUG: " << message << "]" << std::endl;
#else
#define DEBUG_MSG(message)
#endif

// Parameter for cost function, used in Parser::readWeight
# define MIN_WEIGHT 0.0000000001

#define UNSET_IDX -1
#define P_PER_NODE 16

// Parameter for KNN
#define MAX_NEIGHBORS 20
// #define MAX_SQUARE_DISPLACEMENT 100000000
// #define MAX_BANDWIDTH 10000
#define BANDWIDTH_SELECTION_NEIGHBOR 4
#define SHIFT_TOLERANCE 0.1
#define SQUARE_EPSILON 25000000

// OpenMP parallel CPU core
#define MAX_THREADS 8


#define PRO_BAR_LENGTH 50

#include "Coor.h"
#include <string>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <unistd.h>

class Param{
    public:
        Param();
        ~Param();
        double MAX_SQUARE_DISPLACEMENT;
        double MAX_BANDWIDTH;
};

double SquareEuclideanDistance(const Coor &p1, const Coor &p2);
double MangattanDistance(const Coor &p1, const Coor &p2);
double GaussianKernel(const Coor &p1, const Coor &p2, double bandwidth);
double HPWL(const Coor&, const Coor&);
bool IsOverlap(const Coor &coor1, double w1, double h1, const Coor &coor2, double w2, double h2);
std::string toStringWithPrecision(double value, int numAfterDot);
void update_bar(int percent_done);

#endif