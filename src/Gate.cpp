#include "Gate.h"

Gate::Gate(){
    ;
}

Gate::~Gate(){
    ;
}

ostream &operator<<(ostream &out, const Gate &gate){
    out << "Instance Name: " << gate.instanceName << endl;
    out << "Coor: " << gate.coor << endl;
    out << gate.cell << endl;
    return out;
}