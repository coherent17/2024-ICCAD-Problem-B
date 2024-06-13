#include "OptimalLocation.h"

double obj_function::forward(vector<Coor>& in){
    loss = 0;
    int i=0;
    for(auto& net_m : mgr.Net_Map){
        Net& n = net_m.second;
        x_pos[i] = 0;
        x_neg[i] = 0;
        y_pos[i] = 0;
        y_neg[i] = 0;
        for(int j=0;j<n.getNumPins();j++){
            const Pin& p = n.getPin(j);
            string instName = p.getInstanceName();
            double x, y;
            if(mgr.FF_Map.count(instName)){
                x = in[mgr.FF_Map[instName].getIdx()].x;
                y = in[mgr.FF_Map[instName].getIdx()].y;
            }
            else{
                x = mgr.FF_Map[instName].getCoor().x;
                y = mgr.FF_Map[instName].getCoor().y;
            }
            x_pos[i] += exp( x / gamma);
            x_neg[i] += exp(-x / gamma);
            y_pos[i] += exp( y / gamma);
            y_neg[i] += exp(-y / gamma);
        }
        loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
    }
    input_ = in;
    return loss;
}

vector<Coor>& obj_function::backward(){
    for(size_t i=0;i<grad_.size();i++){
        grad_[i].x = 0;
        grad_[i].y = 0;
    }
    int i=0;
    for(auto& net_m : mgr.Net_Map){
        Net& n = net_m.second;
        for(int j=0;j<n.getNumPins();j++){
            const Pin& p = n.getPin(j);
            string instName = p.getInstanceName();
            if(mgr.FF_Map.count(instName)){
                int id = mgr.FF_Map[instName].getIdx();
                grad_[id].x += (exp(input_[id].x / gamma) / x_pos[i]) - (exp(-input_[id].x / gamma) / x_neg[i]);
                grad_[id].y += (exp(input_[id].y / gamma) / y_pos[i]) - (exp(-input_[id].y / gamma) / y_neg[i]);   
            }
        }
        i++;
    }
    return grad_;
}

Gradient::Gradient(obj_function &obj,
                    std::vector<Coor> &var,  
                    const double &alpha)
    : grad_prev_(var.size()),
      dir_prev_(var.size()),
      step_(0),
      alpha_(alpha),
      obj_(obj),
      var_(var) {}

void Gradient::Initialize() {
    step_ = 0;
}

/**
 * @details Update the solution once using the conjugate gradient method.
 */
void Gradient::Step() {
    const size_t &kNumModule = var_.size();

    // Compute the gradient direction
    obj_.forward(var_); // Forward, compute the function value and cache from the input
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
    for (size_t i = 0; i < kNumModule; ++i) {
        var_[i].x = var_[i].x + alpha_ * dir[i].x;
        var_[i].y = var_[i].y + alpha_ * dir[i].y;
    }

    // Update the cache data members
    grad_prev_ = obj_.grad();
    dir_prev_ = dir;
    step_++;
}
