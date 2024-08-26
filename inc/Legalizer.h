#ifndef _LEGALIZER_H_
#define _LEGALIZER_H_

#include "Cell.h"
#include "Node.h"
#include "Row.h"
#include "Subrow.h"
#include "Manager.h"
#include "Timer.h"
#include "Util.h"
#include "XTour.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cfloat>

#ifdef ENABLE_DEBUG_LGZ
#define DEBUG_LGZ(message) std::cout << "[LEGALIZER] " << message << std::endl
#else
#define DEBUG_LGZ(message)
#endif

class Node;
class Row;
class Legalizer{
private:
    Manager& mgr;
    std::vector<Node *> ffs;
    std::vector<Node *> gates;
    std::vector<Row *> rows;
    Timer timer;

public:
    explicit Legalizer(Manager& mgr);
    ~Legalizer();
    void initial();
    void run();
    Coor FindPlace(const Coor &coor, Cell * cell);
    void UpdateRows(FF* newFF);

private:
    // void CheckIfMBFFMove();
    void LoadFF();
    void LoadGate();
    void LoadPlacementRow();
    void SliceRowsByRows();
    void SliceRowsByGate();
    void Tetris();
    void LegalizeWriteBack();
    

    // Helper Function
    static void UpdateXList(double start, double end, std::list<XTour> & xList);
    size_t FindClosestRow(const Coor &coor);
    static int FindClosestSubrow(Node *ff, Row *row);
    void PredictFFLGPlace(const Coor &coor, Cell* cell, size_t row_idx, bool &placeable, double &minDisplacement, Coor &newCoor);
    double PlaceFF(Node *ff, size_t row_idx, bool& placeable);
    bool ContinousAndEmpty(double startX, double startY, double w, double h, int row_idx);
    static double getDisplacement(const Coor &Coor1, const Coor &Coor2);
    
    friend class DetailPlacement;
};

#endif


// Backup
// #include <iostream>
// #include <boost/icl/interval_map.hpp>
// #include <boost/icl/interval.hpp>

// using namespace boost::icl;

// class Legalizer {
// public:
//     void UpdateXInterval(int start, int end, interval_map<int, int>& imap) {
//         // Define the new interval to be inserted, using closed intervals
//         interval<int>::type newInterval = interval<int>::closed(start, end);

//         // Add or merge the interval into the interval_map
//         imap += std::make_pair(newInterval, 1);

//         // Normalize intervals to merge adjacent ones
//         NormalizeIntervals(imap);

//         // Debug output to show the current state of imap after each update
//         for (const auto& elem : imap) {
//             std::cout << "[" << elem.first.lower() << "," << elem.first.upper() << "] -> " << elem.second << std::endl;
//         }
//         std::cout << "\n\n";
//     }

// private:
//     void NormalizeIntervals(interval_map<int, int>& imap) {
//         interval_map<int, int> mergedMap;
//         for (const auto& elem : imap) {
//             if (!mergedMap.empty()) {
//                 auto lastElem = mergedMap.end();
//                 --lastElem;

//                 if (lastElem->first.upper() >= elem.first.lower() - 1) {
//                     // Merge intervals
//                     interval<int>::type mergedInterval = interval<int>::closed(
//                         lastElem->first.lower(),
//                         std::max(lastElem->first.upper(), elem.first.upper())
//                     );
//                     mergedMap.erase(lastElem);
//                     mergedMap += std::make_pair(mergedInterval, 1);
//                 } else {
//                     mergedMap += elem;
//                 }
//             } else {
//                 mergedMap += elem;
//             }
//         }
//         imap = mergedMap;
//     }
// };

// int main() {
//     interval_map<int, int> imap;
//     Legalizer legalizer;

//     // Applying updates with potential overlapping and adjacent intervals
//     legalizer.UpdateXInterval(2, 4, imap);
//     legalizer.UpdateXInterval(6, 12, imap);
//     legalizer.UpdateXInterval(13, 16, imap);
//     legalizer.UpdateXInterval(2, 19, imap);
//     legalizer.UpdateXInterval(5, 8, imap);

//     // Print the final result
//     std::cout << "Final Interval Map:\n";
//     for (const auto& elem : imap) {
//         std::cout << "[" << elem.first.lower() << "," << elem.first.upper() << "] -> " << elem.second << std::endl;
//     }

//     return 0;
// }