#ifndef _LEGALIZER_H_
#define _LEGALIZER_H_

#include "Node.h"
#include <vector>
#include <string.h>

class Legalizer{
private:
    std::vector<Node *> ffs;
    std::vector<Node *> gates;

public:
    Legalizer();
    ~Legalizer();

    void run();
};

#endif