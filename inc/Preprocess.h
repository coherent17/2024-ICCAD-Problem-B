#ifndef PREPROCESS_H
#define PREPROCESS_H
#include "Manager.h"
class FF;
class Manager;
class Preprocess{
    private:
        Manager& mgr;


        // For debanking
        /**
         * @FF_list: map all of the debanking single bit ff name to its pointer
         * @FF_list_Map: map original mbff pin name to debank single bit ff instance name
         */
        std::unordered_map<std::string, FF*> FF_list; // list of all FF, in one bit without physical info.
        std::unordered_map<std::string, std::string> FF_list_Map; // map input MBFF to FF_list, MBFF_NAME/PIN_NAME -> FF_list key

        bool changed;
    public:
        explicit Preprocess(Manager& mgr);
        ~Preprocess();
        void run();
        // get function
        std::unordered_map<std::string, FF*>& getFFList(){
            return FF_list;
        }
        std::unordered_map<std::string, std::string>& getFFListMap(){
            return FF_list_Map;
        }

    private:
        void Debank();
        void Build_Circuit_Gragh();
        void optimal_FF_location();
        void ChangeCell();
        // function for build circuit gragh
        static void findDrivingCell(const Net& n, std::string& driving_cell, std::string& driving_pin, 
                             bool& is_CLK, bool& has_driving_cell);
        void connectNet(const Net& n, std::string& driving_cell, std::string& driving_pin);
        void setLargestOutput(Instance* cur_instance);
        void DelayPropagation();
        void propagaFF(std::queue<Instance*>& q, FF* ff);
        void propagaGate(std::queue<Instance*>& q, Gate* gate);

        // function for optimal ff location
        double getSlackStatistic(bool show); // return TNS
        double updateSlack(std::vector<FF*>& FFs);
};

#endif