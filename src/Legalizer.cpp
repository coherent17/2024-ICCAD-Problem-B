#include "Legalizer.h"

Legalizer::Legalizer(Manager& mgr) : mgr(mgr){}

Legalizer::~Legalizer(){

}

void Legalizer::run(){
    ConstructDB();
    SliceRows();
    row_slicing_unit_test();
}

void Legalizer::ConstructDB(){
    LoadFF();
    LoadGate();
    LoadPlacementRow();
    DEBUG_LGZ("Finish Load Databse to Legalizer");
}

void Legalizer::LoadFF(){
    // Construct FF database
    DEBUG_LGZ("Load FF to Databse");
    numffs = mgr.FF_Map.size();
    assert(mgr.FFs.size() == mgr.FF_Map.size());
    for(size_t i = 0; i < mgr.FFs.size(); i++){
        Node *ff = new Node();
        ff->setName(mgr.FFs[i]->getInstanceName());
        ff->setGPCoor(mgr.FFs[i]->getNewCoor());
        ff->setLGZCoor(mgr.FFs[i]->getNewCoor());
        ff->setW(mgr.FFs[i]->getW());
        ff->setH(mgr.FFs[i]->getH());
        ff->setWeight(mgr.FFs[i]->getPinCount());
        ffs.push_back(ff);
    }
}

void Legalizer::LoadGate(){
    // Construct Gate database
    DEBUG_LGZ("Load Gate to Databse");
    numgates = mgr.Gate_Map.size();
    for(const auto &pair: mgr.Gate_Map){
        Node *gate = new Node();
        gate->setName(pair.second->getInstanceName());
        gate->setGPCoor(pair.second->getCoor());
        gate->setLGZCoor(pair.second->getCoor());
        gate->setW(pair.second->getW());
        gate->setH(pair.second->getH());
        gate->setWeight(pair.second->getPinCount());
        gates.push_back(gate);
    }
}

void Legalizer::LoadPlacementRow(){
    // Construct Placement row database
    DEBUG_LGZ("Load Placement Row to Databse");
    std::vector<PlacementRow> PlacementRows = mgr.die.getPlacementRows();
    for(size_t i = 0; i < PlacementRows.size(); i++){
        Row *row = new Row();
        row->setStartCoor(PlacementRows[i].startCoor);
        row->setSiteHeight(PlacementRows[i].siteHeight);
        row->setSiteWidth(PlacementRows[i].siteWidth);
        row->setNumOfSite(PlacementRows[i].NumOfSites);
        // init the first subrow in row class
        Subrow *subrow = new Subrow();
        subrow->setStartX(PlacementRows[i].startCoor.x);
        subrow->setEndX(PlacementRows[i].startCoor.x + PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setFreeWidth(PlacementRows[i].siteWidth * PlacementRows[i].NumOfSites);
        subrow->setLastCluster(nullptr);
        row->addSubrows(subrow);
        rows.push_back(row);
    }
}

void Legalizer::SliceRows(){
    DEBUG_LGZ("Seperate PlacementRows by Gate Cell");
    std::sort(gates.begin(), gates.end(), [](Node *node1, Node *node2){
        return node1->getGPCoor().x < node2->getGPCoor().x;
    });

    // for(size_t i = 0; i < gates.size(); i++){
    //     DEBUG_LGZ(gates[i]->getName() + "/" + std::to_string(gates[i]->getGPCoor().x));
    // }

    // for each gate, if it occupies a placement row, slice the row
    for(const auto &gate : gates){
        for(auto &row : rows){
            if(IsOverlap(gate->getGPCoor(), gate->getW(), gate->getH(), row->getStartCoor(), row->getSiteWidth() * row->getNumOfSite(), row->getSiteHeight())){
                DEBUG_LGZ("Overlap detected...");
                row->slicing(gate);
            }
        }
    }
}

bool Legalizer::IsOverlap(const Coor &coor1, double w1, double h1, const Coor &coor2, double w2, double h2){
    using namespace boost::geometry;
    typedef model::point<double, 2, cs::cartesian> point;
    typedef model::box<point> box;
    box box1(point(coor1.x, coor1.y), point(coor1.x + w1, coor1.y + h1));
    box box2(point(coor2.x, coor2.y), point(coor2.x + w2, coor2.y + h2));
    return intersects(box1, box2);
}

void Legalizer::row_slicing_unit_test(){
    DEBUG_LGZ("Unit test for Row::slicing(Node *gate)");
    //Testcase for testing Row::slicing()
    // Test Case 1: No overlap
    Row row1;
    row1.setStartCoor(Coor(0, 0));
    row1.setSiteWidth(10);
    row1.setSiteHeight(10);
    row1.setNumOfSite(10);
    Subrow* subrow1 = new Subrow();
    subrow1->setStartX(0);
    subrow1->setEndX(100);
    subrow1->setFreeWidth(100);
    row1.addSubrows(subrow1);
    Node* gate1 = new Node();
    gate1->setGPCoor(Coor(110, 0));
    gate1->setW(20);
    gate1->setH(10);
    row1.slicing(gate1);
    std::cout << row1 << std::endl;
    // Expect the original subrow to remain unchanged

    // Test Case 2: Partial overlap (start only)
    Row row2;
    row2.setStartCoor(Coor(0, 0));
    row2.setSiteWidth(10);
    row2.setSiteHeight(10);
    row2.setNumOfSite(10);
    Subrow* subrow2 = new Subrow();
    subrow2->setStartX(0);
    subrow2->setEndX(100);
    subrow2->setFreeWidth(100);
    row2.addSubrows(subrow2);
    Node* gate2 = new Node();
    gate2->setGPCoor(Coor(0, 0));
    gate2->setW(20);
    gate2->setH(10);
    row2.slicing(gate2);
    std::cout << row2 << std::endl;
    // Expect a new subrow before the gate and another one after

    // Test Case 3: Complete overlap
    Row row3;
    row3.setStartCoor(Coor(0, 0));
    row3.setSiteWidth(10);
    row3.setSiteHeight(10);
    row3.setNumOfSite(10);
    Subrow* subrow3 = new Subrow();
    subrow3->setStartX(0);
    subrow3->setEndX(100);
    subrow3->setFreeWidth(100);
    row3.addSubrows(subrow3);
    Node* gate3 = new Node();
    gate3->setGPCoor(Coor(0, 0));
    gate3->setW(60);
    gate3->setH(10);
    row3.slicing(gate3);
    std::cout << row3 << std::endl;
    // Expect no subrows left

    // Test Case 4: Overlap with middle
    Row row4;
    row4.setStartCoor(Coor(0, 0));
    row4.setSiteWidth(10);
    row4.setSiteHeight(10);
    row4.setNumOfSite(10);
    Subrow* subrow4 = new Subrow();
    subrow4->setStartX(0);
    subrow4->setEndX(100);
    subrow4->setFreeWidth(100);
    row4.addSubrows(subrow4);
    Node* gate4 = new Node();
    gate4->setGPCoor(Coor(30, 0));
    gate4->setW(40);
    gate4->setH(10);
    row4.slicing(gate4);
    std::cout << row4 << std::endl;
    // Expect new subrows before, within, and after the gate

    // Test Case 5: Edge Cases
    Row row5;
    row5.setStartCoor(Coor(0, 0));
    row5.setSiteWidth(10);
    row5.setSiteHeight(10);
    row5.setNumOfSite(10);
    Subrow* subrow5 = new Subrow();
    subrow5->setStartX(0);
    subrow5->setEndX(100);
    subrow5->setFreeWidth(100);
    row5.addSubrows(subrow5);
    Node* gate5 = new Node();
    gate5->setGPCoor(Coor(100, 0));
    gate5->setW(10);
    gate5->setH(10);
    row5.slicing(gate5);
    std::cout << row5 << std::endl;
    // Expect a new subrow with a 10-width section aligned with the site's width

    // Gate width aligns with the site width and the gate starts at a site boundary
    Row row6;
    row6.setStartCoor(Coor(0, 0));
    row6.setSiteWidth(10);
    row6.setSiteHeight(10);
    row6.setNumOfSite(10);
    Subrow* subrow6 = new Subrow();
    subrow6->setStartX(0);
    subrow6->setEndX(100);
    subrow6->setFreeWidth(100);
    row6.addSubrows(subrow6);
    Node* gate6 = new Node();
    gate6->setGPCoor(Coor(30, 0));
    gate6->setW(20);  // Gate width is 20 which is a multiple of siteWidth
    gate6->setH(10);
    row6.slicing(gate6);
    std::cout << row6 << std::endl;
    // Expect subrows before the gate, within the gate, and after the gate, with proper alignment to siteWidth

    // Gate starts and ends at the boundary of two sites
    Row row7;
    row7.setStartCoor(Coor(0, 0));
    row7.setSiteWidth(10);
    row7.setSiteHeight(10);
    row7.setNumOfSite(10);
    Subrow* subrow7 = new Subrow();
    subrow7->setStartX(0);
    subrow7->setEndX(100);
    subrow7->setFreeWidth(100);
    row7.addSubrows(subrow7);
    Node* gate7 = new Node();
    gate7->setGPCoor(Coor(10, 0));  // Gate starts on site boundary
    gate7->setW(30);  // Gate width crosses the boundary of a single site
    gate7->setH(10);
    row7.slicing(gate7);
    std::cout << row7 << std::endl;
    // Expect new subrows before the gate, within the gate, and after the gate with proper alignment

    // Gate starts exactly where a site ends
    Row row8;
    row8.setStartCoor(Coor(0, 0));
    row8.setSiteWidth(10);
    row8.setSiteHeight(10);
    row8.setNumOfSite(10);
    Subrow* subrow8 = new Subrow();
    subrow8->setStartX(0);
    subrow8->setEndX(100);
    subrow8->setFreeWidth(100);
    row8.addSubrows(subrow8);
    Node* gate8 = new Node();
    gate8->setGPCoor(Coor(90, 0));  // Gate starts at the end of the 9th site
    gate8->setW(20);  // Gate width overlaps with two sites
    gate8->setH(10);
    row8.slicing(gate8);
    std::cout << row8 << std::endl;
    // Expect a new subrow before the gate and one or two new subrows after the gate, properly aligned

    // Gate is smaller than the site width and aligned with the grid
    Row row9;
    row9.setStartCoor(Coor(0, 0));
    row9.setSiteWidth(10);
    row9.setSiteHeight(10);
    row9.setNumOfSite(10);
    Subrow* subrow9 = new Subrow();
    subrow9->setStartX(0);
    subrow9->setEndX(100);
    subrow9->setFreeWidth(100);
    row9.addSubrows(subrow9);
    Node* gate9 = new Node();
    gate9->setGPCoor(Coor(45, 0));  // Gate is in the middle of a site
    gate9->setW(5);  // Gate width is smaller than siteWidth
    gate9->setH(10);
    row9.slicing(gate9);
    std::cout << row9 << std::endl;
    // Expect new subrows before and after the gate, aligned with the site width

    // Multiple gates overlapping the same subrow
    Row row10;
    row10.setStartCoor(Coor(0, 0));
    row10.setSiteWidth(10);
    row10.setSiteHeight(10);
    row10.setNumOfSite(10);
    Subrow* subrow10 = new Subrow();
    subrow10->setStartX(0);
    subrow10->setEndX(100);
    subrow10->setFreeWidth(100);
    row10.addSubrows(subrow10);

    Node* gate10a = new Node();
    gate10a->setGPCoor(Coor(20, 0));
    gate10a->setW(15);
    gate10a->setH(10);

    Node* gate10b = new Node();
    gate10b->setGPCoor(Coor(60, 0));
    gate10b->setW(25);
    gate10b->setH(10);

    // Slice the row with the first gate
    row10.slicing(gate10a);
    // Slice the row again with the second gate
    row10.slicing(gate10b);

    std::cout << row10 << std::endl;
    // Expect new subrows before, within, and after the gates, with proper alignment

    // Small gate placed exactly at the edge of the first site
    Row row11;
    row11.setStartCoor(Coor(0, 0));
    row11.setSiteWidth(10);
    row11.setSiteHeight(10);
    row11.setNumOfSite(10);
    Subrow* subrow11 = new Subrow();
    subrow11->setStartX(0);
    subrow11->setEndX(100);
    subrow11->setFreeWidth(100);
    row11.addSubrows(subrow11);
    Node* gate11 = new Node();
    gate11->setGPCoor(Coor(0, 0));  // Gate starts at the beginning of the first site
    gate11->setW(5);  // Small gate width
    gate11->setH(10);
    row11.slicing(gate11);
    std::cout << row11 << std::endl;
    // Expect new subrows to the right of the gate, properly aligned

    // Complex overlap scenario to check correct handling of alignment
    Row row12;
    row12.setStartCoor(Coor(0, 0));
    row12.setSiteWidth(10);
    row12.setSiteHeight(10);
    row12.setNumOfSite(10);
    Subrow* subrow12 = new Subrow();
    subrow12->setStartX(0);
    subrow12->setEndX(100);
    subrow12->setFreeWidth(100);
    row12.addSubrows(subrow12);
    Node* gate12 = new Node();
    gate12->setGPCoor(Coor(35, 0));  // Gate starts at the middle of a site
    gate12->setW(30);  // Gate width spans across several sites
    gate12->setH(10);
    row12.slicing(gate12);
    std::cout << row12 << std::endl;
    // Expect subrows before the gate, within the gate, and after the gate, properly aligned
}