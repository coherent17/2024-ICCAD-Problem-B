#ifndef OPTIMAL_LOACTION_H
#define OPTIMAL_LOCATION_H

#include "Manager.h"
using std::cout;
using std::endl;
using std::vector;
using std::string;
using std::unordered_map;

class FF;
class obj_function{
    private:
        vector<Coor> input_;
        vector<Coor> grad_;
        unordered_map<string, int>& idx_map;
        double loss;
        Manager& mgr;
        double gamma;
        vector<double> x_pos, x_neg, y_pos, y_neg;
    public:
        obj_function(Manager&mgr, unordered_map<string, int>& idx_map);
        ~obj_function(){
        }
        double forward();
        vector<Coor>& backward();
        const vector<Coor>& grad()const {return grad_;}
};

class Gradient{
   public:
    /////////////////////////////////
    // Constructors
    /////////////////////////////////

    Gradient(Manager&mgr, obj_function &obj, std::vector<Coor> &var, const double &alpha, unordered_map<string, int>& idx_map);
    ~Gradient(){
    }
    /////////////////////////////////
    // Methods
    /////////////////////////////////

    // Initialize the optimizer
    void Initialize();

    // Perform one optimization step
    void Step();

   private:
    /////////////////////////////////
    // Data members
    /////////////////////////////////

    vector<Coor> grad_prev_;  // Gradient of the objective function at the previous
                                             // step, i.e., g_{k-1} in the NTUPlace3 paper
    vector<Coor> dir_prev_;   // Direction of the previous step,
                                             // i.e., d_{k-1} in the NTUPlace3 paper
    size_t step_;                            // Current step number
    double alpha_;                           // Step size
    obj_function &obj_;                 // Objective function to optimize
    vector<Coor> &var_;  // Variables to optimize
    unordered_map<string, int>& idx_map;
    Manager& mgr;
};


#endif