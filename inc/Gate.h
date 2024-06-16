#ifndef _GATE_H_
#define _GATE_H_

#include "Cell.h"

class Cell;

class Gate : public Instance{
private:

public:
    Gate();
    ~Gate();

    friend ostream &operator<<(ostream &, const Gate &);
};

#endif
