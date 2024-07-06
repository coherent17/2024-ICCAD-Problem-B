#include "OptimalLocation.h"

obj_function::obj_function(Manager&mgr, unordered_map<string, int>& idx_map):idx_map(idx_map), mgr(mgr), x_pos(mgr.FF_list.size()*2), x_neg(mgr.FF_list.size()*2), y_pos(mgr.FF_list.size()*2), y_neg(mgr.FF_list.size()*2){
    gamma = mgr.die.getDieBorder().y * 0.01;
    grad_ = vector<Coor>(mgr.FF_Map.size());
}

double obj_function::forward(){
    loss = 0;
    int i=0;
    for(auto& FF_m : mgr.FF_list){
        FF* cur_ff = FF_m.second;
        // input net
        x_pos[i] = 0;
        x_neg[i] = 0;
        y_pos[i] = 0;
        y_neg[i] = 0;
        assert(cur_ff->getInputInstances().size() == 1 && "FF input should only drive by one instance");
        std::string inputInstanceName = cur_ff->getInputInstances()[0].first;
        std::string inputPinName = cur_ff->getInputInstances()[0].second;
        Coor inputCoor;
        if(mgr.IO_Map.count(inputInstanceName)){
            inputCoor = mgr.IO_Map[inputInstanceName].getCoor();
        }
        else if(mgr.Gate_Map.count(inputInstanceName)){
            inputCoor = mgr.Gate_Map[inputInstanceName]->getCoor() + mgr.Gate_Map[inputInstanceName]->getPinCoor(inputPinName);
        }
        else{
            inputCoor = mgr.FF_list[inputInstanceName]->getCoor();
        }
        Coor curCoor = cur_ff->getCoor();
        x_pos[i] = exp( inputCoor.x / gamma) + exp( curCoor.x / gamma);
        x_neg[i] = exp(-inputCoor.x / gamma) + exp(-curCoor.x / gamma);
        y_pos[i] = exp( inputCoor.y / gamma) + exp( curCoor.y / gamma);
        y_neg[i] = exp(-inputCoor.y / gamma) + exp(-curCoor.y / gamma);
        loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
        i++;
        // output net (only for the ff output to IO, to avoid double calculation)
        Instance* outputInstance = cur_ff->getLargestOutput().first;
        if(outputInstance){
            std::string outputInstanceName = outputInstance->getInstanceName();
            std::string outputPinName = cur_ff->getLargestOutput().second;
            Coor outputCoor;
            if(mgr.IO_Map.count(outputInstanceName)){
                outputCoor = mgr.IO_Map[outputInstanceName].getCoor();
            }
            else if(mgr.Gate_Map.count(outputInstanceName)){
                outputCoor = mgr.Gate_Map[outputInstanceName]->getCoor() + mgr.Gate_Map[outputInstanceName]->getPinCoor(outputPinName);
            }
            else{
                outputCoor = mgr.FF_list[outputInstanceName]->getCoor();
            }
            x_pos[i] = exp( outputCoor.x / gamma) + exp( curCoor.x / gamma);
            x_neg[i] = exp(-outputCoor.x / gamma) + exp(-curCoor.x / gamma);
            y_pos[i] = exp( outputCoor.y / gamma) + exp( curCoor.y / gamma);
            y_neg[i] = exp(-outputCoor.y / gamma) + exp(-curCoor.y / gamma);
            loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
            i++;
        }
    }

    // for(auto& net_m : mgr.Net_Map){
    //     Net& n = net_m.second;
    //     x_pos[i] = 0;
    //     x_neg[i] = 0;
    //     y_pos[i] = 0;
    //     y_neg[i] = 0;
    //     for(int j=0;j<n.getNumPins();j++){
    //         const Pin& p = n.getPin(j);
    //         string instName = p.getInstanceName();
    //         double x, y;
    //         if(mgr.FF_Map.count(instName)){
    //             x = in[mgr.FF_Map[instName].getIdx()].x;
    //             y = in[mgr.FF_Map[instName].getIdx()].y;
    //         }
    //         else{
    //             x = mgr.FF_Map[instName].getCoor().x;
    //             y = mgr.FF_Map[instName].getCoor().y;
    //         }
    //         x_pos[i] += exp( x / gamma);
    //         x_neg[i] += exp(-x / gamma);
    //         y_pos[i] += exp( y / gamma);
    //         y_neg[i] += exp(-y / gamma);
    //     }
    //     loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
    //     i++;
    // }
    // input_ = in;
    cout << "loss : " << loss << std::endl;
    return loss;
}

vector<Coor>& obj_function::backward(){
    for(size_t i=0;i<grad_.size();i++){
        grad_[i].x = 0;
        grad_[i].y = 0;
    }
    int i=0;
    for(auto& FF_m : mgr.FF_list){
        FF* cur_ff = FF_m.second;
        int idx = idx_map[cur_ff->getInstanceName()];
        Coor curCoor = cur_ff->getCoor();
        grad_[idx].x += (exp(curCoor.x / gamma) / x_pos[i]) - (exp(-curCoor.x / gamma) / x_neg[i]);
        grad_[idx].y += (exp(curCoor.y / gamma) / y_pos[i]) - (exp(-curCoor.y / gamma) / y_neg[i]);   
        i++;

        Instance* outputInstance = cur_ff->getLargestOutput().first;
        if(outputInstance){
            grad_[idx].x += (exp(curCoor.x / gamma) / x_pos[i]) - (exp(-curCoor.x / gamma) / x_neg[i]);
            grad_[idx].y += (exp(curCoor.y / gamma) / y_pos[i]) - (exp(-curCoor.y / gamma) / y_neg[i]); 
            i++;
        }
    }
    // for(auto& net_m : mgr.Net_Map){
    //     Net& n = net_m.second;
    //     for(int j=0;j<n.getNumPins();j++){
    //         const Pin& p = n.getPin(j);
    //         string instName = p.getInstanceName();
    //         if(mgr.FF_Map.count(instName)){
    //             int id = mgr.FF_Map[instName].getIdx();
    //             grad_[id].x += (exp(input_[id].x / gamma) / x_pos[i]) - (exp(-input_[id].x / gamma) / x_neg[i]);
    //             grad_[id].y += (exp(input_[id].y / gamma) / y_pos[i]) - (exp(-input_[id].y / gamma) / y_neg[i]);   
    //         }
    //     }
    //     i++;
    // }
    return grad_;
}

Gradient::Gradient( Manager &mgr,
                    obj_function &obj,
                    std::vector<Coor> &var,  
                    const double &alpha,
                    unordered_map<string, int>& idx_map)
    : grad_prev_(var.size()),
      dir_prev_(var.size()),
      step_(0),
      alpha_(alpha),
      obj_(obj),
      var_(var),
      idx_map(idx_map),
      mgr(mgr) {}

void Gradient::Initialize() {
    step_ = 0;
}

/**
 * @details Update the solution once using the conjugate gradient method.
 */
void Gradient::Step() {
    const size_t &kNumModule = var_.size();

    // Compute the gradient direction
    obj_.forward(); // Forward, compute the function value and cache from the input
    obj_.backward();  // Backward, compute the gradient according to the cache

    // Compute the Polak-Ribiere coefficient and conjugate directions
    double beta;                                  // Polak-Ribiere coefficient
    std::vector<Coor> dir(kNumModule);  // conjugate directions
    if (step_ == 0) {
        // For the first step, we will set beta = 0 and d_0 = -g_0
        beta = 0.;
        for (size_t i = 0; i < kNumModule; ++i) {
            dir[i].x = -obj_.grad().at(i).x;
            dir[i].y = -obj_.grad().at(i).y;
        }
    } else {
        // For the remaining steps, we will calculate the Polak-Ribiere coefficient and
        // conjugate directions normally
        double t1 = 0.;  // Store the numerator of beta
        double t2 = 0.;  // Store the denominator of beta
        for (size_t i = 0; i < kNumModule; ++i) {
            Coor g = obj_.grad().at(i);
            Coor t3;
            t3.x = g.x * (g.x - grad_prev_.at(i).x);
            t3.y = g.y * (g.y - grad_prev_.at(i).y);
            t1 += t3.x + t3.y;
            t2 += std::abs(g.x) + std::abs(g.y);
        }
        beta = t1 / (t2 * t2);
        for (size_t i = 0; i < kNumModule; ++i) {
            dir[i].x = -obj_.grad().at(i).x + beta * dir_prev_.at(i).x;
            dir[i].y = -obj_.grad().at(i).y + beta * dir_prev_.at(i).y;
        }
    }

    // Assume the step size is constant
    // TODO(Optional): Change to dynamic step-size control

    // Update the solution
    // Please be aware of the updating directions, i.e., the sign for each term.
    for(auto& ff_m : mgr.FF_list){
        int idx = idx_map[ff_m.second->getInstanceName()];
        FF* ff = ff_m.second;
        Coor coor = ff->getCoor();
        coor.x = coor.x + alpha_ * dir[idx].x;
        coor.y = coor.y + alpha_ * dir[idx].y;
        ff->setCoor(coor);
    }
    // for (size_t i = 0; i < kNumModule; ++i) {
    //     var_[i].x = var_[i].x + alpha_ * dir[i].x;
    //     var_[i].y = var_[i].y + alpha_ * dir[i].y;
    // }

    // Update the cache data members
    grad_prev_ = obj_.grad();
    dir_prev_ = dir;
    step_++;
}
