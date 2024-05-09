#include "Util.h"

std::ostream& operator<<(std::ostream& os, const Coor& coor) {
    os << "(" << coor.first << ", " << coor.second << ")";
    return os;
}