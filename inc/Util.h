#ifndef _UTIL_H_
#define _UTIL_H_

#define DEBUG_MSG(message) cout << "[DEBUG: " << message << "]" << endl;

#include "Coor.h"
#include "Pin.h"

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cassert>
#include <vector>
#include <fstream>
#include <cmath>
#include <climits>
#include <limits.h>
using namespace std;

double EuclideanDistance(const Coor &, const Coor &);
double MangattanDistance(const Coor &, const Coor &);
string getNewFFName(const string&, const int&); // given prefix string and counter return the concate name = (prefix + string(int))
// double HPWL(const Net& n, vector<Coor>& c, Manager& mgr);
#endif