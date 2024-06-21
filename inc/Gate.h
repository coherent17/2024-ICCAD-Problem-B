#ifndef _GATE_H_
#define _GATE_H_

#include <iostream>
#include "Instance.h"


class Gate : public Instance{
private:

public:
    Gate();
    ~Gate();

    friend std::ostream &operator<<(std::ostream &os, const Gate &gate);
};

#endif
