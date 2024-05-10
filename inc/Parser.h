#ifndef _PARSER_H_
#define _PARSER_H_

#include "Manager.h"
#include <fstream>
#include <string>
#include <cassert>
using namespace std;

class Manager;

class Parser{
private:
    ifstream fin;

public:
    Parser(const string &);
    ~Parser();
    void parse(Manager &);

private:
    void readWeight(Manager &);
    void readDieBorder(Manager &);
    void readIOCoordinate(Manager &);
    void readCellLibrary(Manager &);
    void readInstance(Manager &);
    void readNet(Manager &);
    void readBin(Manager &);
    void readPlacementRows(Manager &);
    void readQpinDelay(Manager &);
    void readTimingSlack(Manager &);
    void readGatePower(Manager &);
};

#endif