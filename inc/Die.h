#ifndef _DIE_H_
#define _DIE_H_

#include "Util.h"

class Die{
private:
    Coor DieOrigin;
    Coor DieBorder;
    double BinWidth;
    double BinHeight;
    double BinMaxUtil;

public:
    Die();
    ~Die();
};

#endif