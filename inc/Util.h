#ifndef _UTIL_H_
#define _UTIL_H_

#define DEBUG_MSG(message) cout << "[DEBUG: " << message << "]" << endl;

#include "Coor.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cassert>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

double EuclideanDistance(const Coor &, const Coor &);
double MangattanDistance(const Coor &, const Coor &);

#endif