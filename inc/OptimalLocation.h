#ifndef OPTIMAL_LOACTION_H
#define OPTIMAL_LOCATION_H

#include "Manager.h"

class FF;
class objFunction{
    protected:
        Manager& mgr;
        std::unordered_map<std::string, FF*>& FF_list;
        std::unordered_map<string, int>& idx_map;
        vector<Coor> grad_;
        double loss;
        double gamma;
        vector<vector<double>> x_pos, x_neg, y_pos, y_neg;
        std::vector<FF*>& FFs;
    public:
        objFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, 
                    std::unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs);
        virtual double forward() = 0;
        virtual vector<Coor>& backward(int step, bool onlyNegative) = 0;
        virtual const vector<Coor>& grad()const {return grad_;}
        virtual void getWeight(FF* cur_ff, vector<double>& weight) = 0;
};

class preprocessObjFunction : public objFunction{
    public:
        preprocessObjFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, 
                            std::unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs);
        ~preprocessObjFunction();
        double forward() override;
        vector<Coor>& backward(int step, bool onlyNegative) override;
        void getWeight(FF* cur_ff, vector<double>& weight) override;
};

class postBankingObjFunction : public objFunction{
    public:
        postBankingObjFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, 
                            std::unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs);
        ~postBankingObjFunction();
        double forward() override;
        vector<Coor>& backward(int step, bool onlyNegative) override;
        void getWeight(FF* MBFF, vector<double>& weight) override;
};

class Gradient{
    private:
    vector<Coor> grad_prev_;  // Gradient of the objective function at the previous
                                             // step, i.e., g_{k-1} in the NTUPlace3 paper
    vector<Coor> dir_prev_;   // Direction of the previous step,
                                             // i.e., d_{k-1} in the NTUPlace3 paper
    size_t step_;                            // Current step number
    double alpha_;                           // Step size
    size_t kNumModule;
    objFunction &obj_;                 // Objective function to optimize
    std::vector<Coor> dir;
    std::unordered_map<string, int>& idx_map;
    Manager& mgr;
    std::unordered_map<std::string, FF*>& FF_list;
    std::vector<FF*>& FFs;

   public:
    Gradient(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, 
            objFunction &obj, const double &alpha, 
            std::unordered_map<string, int>& idx_map, size_t kNumModule, std::vector<FF*>& FFs);
    ~Gradient();

    void Initialize(double);
    void Step(bool);
};

#endif