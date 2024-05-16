#include "Cell_Library.h"

Cell_Library::Cell_Library(){
    ;
}

Cell_Library::~Cell_Library(){
    ;
}

void Cell_Library::addCell(const string &cellName, const Cell &c){
    Cell_Map[cellName] = c;
}

Cell &Cell_Library::getCellRef(const string &cellName){
    return Cell_Map[cellName];
}

Cell Cell_Library::getCellCopy(const string &cellName)const{
    auto it = Cell_Map.find(cellName);
    if (it != Cell_Map.end()) {
        return it->second; // Return a copy of the found cell
    } else {
        // Handle case where the cell name is not found
        // For now, returning a default-constructed Cell
        return Cell();
    }
}

bool Cell_Library::isFF(const string &cellName){
    return Cell_Map[cellName].getisFF();
}

ostream &operator<<(ostream &out, const Cell_Library &c){
    for(const auto &pair : c.Cell_Map){
        out << pair.second << endl;
    }
    return out;
}