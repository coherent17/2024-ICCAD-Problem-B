#include "Cell_Library.h"

Cell_Library::Cell_Library(){}

Cell_Library::~Cell_Library(){
    for(auto &pair : cellMap){
        delete pair.second;
    }
}

// Setter
void Cell_Library::addCell(const std::string &cellName, Cell *cell){
    cellMap[cellName] = cell;
}

// Getter
Cell *const Cell_Library::getCell(const std::string &cellName)const{
    auto it = cellMap.find(cellName);
    if (it == cellMap.end()) {
        throw std::out_of_range("Cell name not found");
    }
    return it->second;
}

std::unordered_map<std::string, Cell *> Cell_Library::getCellMap()const{
    return this->cellMap;
}

bool Cell_Library::isFF(const std::string &cellName){
    return (cellMap[cellName]->getType() == Cell_Type::FF);
}

std::ostream &operator<<(std::ostream &os, const Cell_Library &cell_library){
    os << "Cell Library: \n";
    for (const auto& pair : cell_library.cellMap) {
        os << *(pair.second) << "\n";
    }
    return os;
}