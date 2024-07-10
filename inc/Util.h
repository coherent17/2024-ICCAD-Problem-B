#ifndef _UTIL_H_
#define _UTIL_H_

#define DEBUG_MSG(message) std::cout << "[DEBUG: " << message << "]" << std::endl;
#define UNSET_IDX -1
#define P_PER_NODE 16

// Parameter for KNN
#define MAX_NEIGHBORS 20
#define MAX_SQUARE_DISPLACEMENT 100000000
#define MAX_BANDWIDTH 10000
#define BANDWIDTH_SELECTION_NEIGHBOR 4
#define SHIFT_TOLERANCE 0.1
#define SQUARE_EPSILON 25000000

// OpenMP parallel CPU core
#define MAX_THREADS 8

#include "Coor.h"
// #include "Pin.h"

// #include <iostream>
// #include <unordered_map>
// #include <unordered_set>
// #include <string>
// #include <cassert>
// #include <vector>
// #include <fstream>
#include <cmath>
// #include <climits>
// #include <queue>
// #include <limits.h>
// using namespace std;

double SquareEuclideanDistance(const Coor &p1, const Coor &p2);
double MangattanDistance(const Coor &p1, const Coor &p2);
double GaussianKernel(const Coor &p1, const Coor &p2, double bandwidth);
// string getNewFFName(const string&, const int&); // given prefix string and counter return the concate name = (prefix + string(int))
double HPWL(const Coor&, const Coor&);
// // double HPWL(const Net& n, vector<Coor>& c, Manager& mgr);

// FF* bank(Coor newbankCoor, cell* bankCellType, std::vector<FF*> ffToBank); // given the new coor(left down) and cell type, it will bank the ff in vector
// // remove FF in vector in FF_Map
// // insert new FF in FF_Map
// // may use marriage algorithm
// std::vector<FF*> debank(FF*, cell*); // given and MBFF, it will debank it into N bit single FF
// // remove input ff from FF_Map
// // insert new in FF_Map
// // new ff coor -> equal D pin coor
// void getNS(double& TNS, double& WNS);
// // ff method to update slack (you can call after move a bank ff or single bit ff)
// // change celltype

#endif