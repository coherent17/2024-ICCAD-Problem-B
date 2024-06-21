#ifndef _PARSER_H_
#define _PARSER_H_

#include <iostream>
#include <fstream>
#include <string>
#include "Manager.h"

class Manager;

class Parser{
private:
    std::ifstream fin;

public:
    explicit Parser(const std::string &filename);
    ~Parser();
    void parse(Manager &mgr);

private:
    void readWeight(Manager &mgr);
    void readDieBorder(Manager &mgr);
    void readIOCoordinate(Manager &mgr);
    void readCellLibrary(Manager &mgr);
    void readInstance(Manager &mgr);
    void readNet(Manager &mgr);
    void readBin(Manager &mgr);
    void readPlacementRows(Manager &mgr);
    void readQpinDelay(Manager &mgr);
    void readTimingSlack(Manager &mgr);
    void readGatePower(Manager &mgr);
    static std::string getSubStringAfterSlash(const std::string &str);
    static std::string getSubStringBeforeSlash(const std::string &str);
};

#endif