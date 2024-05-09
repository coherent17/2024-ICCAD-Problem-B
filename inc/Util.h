#ifndef _UTIL_H_
#define _UTIL_H_

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <utility>

using namespace std;
typedef pair<double, double> Coor;
ostream &operator<<(ostream &os, const Coor &coor);
// math function define in here:
// Square Euclidean Distance
// Manhanttan Distance
// Gaussian (for ff density surface)
// Downstream (for comb cell density surface)

#endif