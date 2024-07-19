#include "OptimalLocation.h"

objFunction::objFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs)
    : mgr(mgr), FF_list(FF_list), idx_map(idx_map),
    x_pos(totalFF*2), x_neg(totalFF*2), 
    y_pos(totalFF*2), y_neg(totalFF*2),
    FFs(FFs){
    gamma = mgr.die.getDieBorder().y * 0.01;
}

preprocessObjFunction::preprocessObjFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs)
    : objFunction(mgr, FF_list, idx_map, totalFF, FFs){
    gamma = mgr.die.getDieBorder().y * 0.01;
    grad_ = vector<Coor>(FF_list.size());
}

preprocessObjFunction::~preprocessObjFunction(){

}

double preprocessObjFunction::forward(){
    loss = 0;
    int i=0;
    for(auto& FF_m : FF_list){
        FF* cur_ff = FF_m.second;
        // input net
        x_pos[i] = 0;
        x_neg[i] = 0;
        y_pos[i] = 0;
        y_neg[i] = 0;
        assert(cur_ff->getInputInstances().size() == 1 && "FF input should only drive by one instance");
        std::string inputInstanceName = cur_ff->getInputInstances()["D"][0].first;
        std::string inputPinName = cur_ff->getInputInstances()["D"][0].second;
        Coor inputCoor;
        if(mgr.IO_Map.count(inputInstanceName)){
            inputCoor = mgr.IO_Map[inputInstanceName].getCoor();
        }
        else if(mgr.Gate_Map.count(inputInstanceName)){
            inputCoor = mgr.Gate_Map[inputInstanceName]->getCoor() + mgr.Gate_Map[inputInstanceName]->getPinCoor(inputPinName);
        }
        else{
            inputCoor = FF_list[inputInstanceName]->getOriginalQ();
        }
        Coor curCoor = cur_ff->getOriginalD();
        x_pos[i] = exp( inputCoor.x / gamma) + exp( curCoor.x / gamma);
        x_neg[i] = exp(-inputCoor.x / gamma) + exp(-curCoor.x / gamma);
        y_pos[i] = exp( inputCoor.y / gamma) + exp( curCoor.y / gamma);
        y_neg[i] = exp(-inputCoor.y / gamma) + exp(-curCoor.y / gamma);
        loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
        i++;
        // output net
        const vector<NextStage>& nextStage = cur_ff->getNextStage();
        if(nextStage.size()){
            // cout << nextStage.size() << endl;
            for(auto& next_p : nextStage){
                std::string outputInstanceName;
                if(next_p.outputGate)
                    outputInstanceName = next_p.outputGate->getInstanceName();
                else // shift register
                    outputInstanceName = next_p.ff->getInstanceName();
                std::string outputPinName = next_p.pinName;
                curCoor = cur_ff->getOriginalQ();
                Coor outputCoor;
                if(mgr.IO_Map.count(outputInstanceName)){
                    outputCoor = mgr.IO_Map[outputInstanceName].getCoor();
                }
                else if(mgr.Gate_Map.count(outputInstanceName)){
                    outputCoor = mgr.Gate_Map[outputInstanceName]->getCoor() + mgr.Gate_Map[outputInstanceName]->getPinCoor(outputPinName);
                }
                else{
                    outputCoor = FF_list[outputInstanceName]->getOriginalD();
                }
                x_pos[i] = exp( outputCoor.x / gamma) + exp( curCoor.x / gamma);
                x_neg[i] = exp(-outputCoor.x / gamma) + exp(-curCoor.x / gamma);
                y_pos[i] = exp( outputCoor.y / gamma) + exp( curCoor.y / gamma);
                y_neg[i] = exp(-outputCoor.y / gamma) + exp(-curCoor.y / gamma);
                loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
                i++;
            }
        }
    }
    return loss;
}

vector<Coor>& preprocessObjFunction::backward(int step, bool onlyNegative){
    for(size_t i=0;i<grad_.size();i++){
        grad_[i].x = 0;
        grad_[i].y = 0;
    }
    int i=0;
    int idx=0;
    for(auto& FF_m : FF_list){
        FF* cur_ff = FF_m.second;
        // weigt by slack
        const vector<NextStage>& nextStage = cur_ff->getNextStage();
        vector<double> weight(1 + nextStage.size());
        getWeight(cur_ff, weight);

        // net of D pin
        Coor curCoor = cur_ff->getOriginalD();
        grad_[idx].x += weight[0] * ((exp(curCoor.x / gamma) / x_pos[i]) - (exp(-curCoor.x / gamma) / x_neg[i]));
        grad_[idx].y += weight[0] * ((exp(curCoor.y / gamma) / y_pos[i]) - (exp(-curCoor.y / gamma) / y_neg[i]));   
        i++;
        // net of Q pin
        curCoor = cur_ff->getOriginalQ();
        for(size_t j=1;j<weight.size();j++){
            grad_[idx].x += weight[j] * ((exp(curCoor.x / gamma) / x_pos[i]) - (exp(-curCoor.x / gamma) / x_neg[i]));
            grad_[idx].y += weight[j] * ((exp(curCoor.y / gamma) / y_pos[i]) - (exp(-curCoor.y / gamma) / y_neg[i])); 
            i++;
        }
        idx++;
    }
    return grad_;
}

void preprocessObjFunction::getWeight(FF* cur_ff, vector<double>& weight){
    // weigt by slack
    double D_slack = cur_ff->getTimingSlack("D");
    const vector<NextStage>& nextStage = cur_ff->getNextStage();
    double sum = 0.0000001;
    // get total
    if(D_slack < 0)
        sum += D_slack;
    for(size_t j=1;j<weight.size();j++){
        if(nextStage[j-1].ff->getTimingSlack("D") < 0)
            sum += nextStage[j-1].ff->getTimingSlack("D");
    }
    // get weight
    if(D_slack < 0)
        weight[0] = D_slack / sum;
    else
        weight[0] = 0;
    for(size_t j=1;j<weight.size();j++){
        if(nextStage[j-1].ff->getTimingSlack("D") < 0)
            weight[j] = nextStage[j-1].ff->getTimingSlack("D") / sum;
        else
            weight[j] = 0;
    }
}

Gradient::Gradient( Manager &mgr,
                    std::unordered_map<std::string, FF*>& FF_list,
                    objFunction &obj,
                    const double &alpha,
                    unordered_map<string, int>& idx_map,
                    size_t kNumModule,
                    std::vector<FF*>& FFs)
    : grad_prev_(kNumModule),
      dir_prev_(kNumModule),
      step_(0),
      alpha_(alpha),
      kNumModule(kNumModule),
      obj_(obj),
      dir(kNumModule),
      idx_map(idx_map),
      mgr(mgr),
      FF_list(FF_list),
      FFs(FFs){
    Initialize(alpha);
}

Gradient::~Gradient(){

}

void Gradient::Initialize(double kAlpha) {
    step_ = 0;
    alpha_ = kAlpha;
}

void Gradient::Step(bool onlyNegative) {
    // Compute the gradient direction
    obj_.forward(); // Forward, compute the function value and cache from the input
    obj_.backward(step_, onlyNegative);  // Backward, compute the gradient according to the cache
    // Compute the Polak-Ribiere coefficient and conjugate directions
    double beta;                                  // Polak-Ribiere coefficient
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
        beta = t1 / (t2 * t2 + 0.00001);
        for (size_t i = 0; i < kNumModule; ++i) {
            dir[i].x = -obj_.grad().at(i).x + beta * dir_prev_.at(i).x;
            dir[i].y = -obj_.grad().at(i).y + beta * dir_prev_.at(i).y;
        }
    }
    // Assume the step size is constant
    // TODO(Optional): Change to dynamic step-size control

    // Update the solution
    // Please be aware of the updating directions, i.e., the sign for each term.
    // int idx=0;
    // for(auto& ff_m : FF_list){
    // #pragma omp parallel for num_threads(MAX_THREADS)
    for(size_t idx=0;idx<kNumModule;idx++){
        // FF* ff = ff_m.second;
        FF* ff = FFs[idx];
        Coor coor = ff->getCoor();
        coor.x = coor.x + alpha_ * dir[idx].x;
        coor.y = coor.y + alpha_ * dir[idx].y;
        ff->setCoor(coor);
        ff->setNewCoor(coor);
    }

    // Update the cache data members
    grad_prev_ = obj_.grad();
    dir_prev_ = dir;
    step_++;
}

postBankingObjFunction::postBankingObjFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs)
    : objFunction(mgr, FF_list, idx_map, totalFF, FFs){
    gamma = mgr.die.getDieBorder().y * 0.01;
    grad_ = vector<Coor>(mgr.FF_Map.size());
}

postBankingObjFunction::~postBankingObjFunction(){

}

double postBankingObjFunction::forward(){
    loss = 0;
    int i=0;
    for(auto& FF_m : FF_list){
        FF* MBFF = FF_m.second;
        size_t bit = MBFF->getCell()->getBits();
        for(size_t j=0;j<bit;j++){
            FF* cur_ff = MBFF->getClusterFF()[j];
            // input net
            x_pos[i] = 0;
            x_neg[i] = 0;
            y_pos[i] = 0;
            y_neg[i] = 0;
            assert(cur_ff->getInputInstances().size() == 1 && "FF input should only drive by one instance");
            std::string inputInstanceName = cur_ff->getInputInstances()["D"][0].first;
            std::string inputPinName = cur_ff->getInputInstances()["D"][0].second;
            Coor inputCoor;
            if(mgr.IO_Map.count(inputInstanceName)){
                inputCoor = mgr.IO_Map[inputInstanceName].getCoor();
            }
            else if(mgr.Gate_Map.count(inputInstanceName)){
                inputCoor = mgr.Gate_Map[inputInstanceName]->getCoor() + mgr.Gate_Map[inputInstanceName]->getPinCoor(inputPinName);
            }
            else{
                FF* inputFF = mgr.preprocessor->getFFList()[inputInstanceName];
                inputCoor = inputFF->physicalFF->getNewCoor() + inputFF->physicalFF->getPinCoor("Q" + inputFF->getPhysicalPinName());
            }
            Coor curCoor = cur_ff->physicalFF->getNewCoor() + cur_ff->physicalFF->getPinCoor("D" + cur_ff->getPhysicalPinName());
            x_pos[i] = exp( inputCoor.x / gamma) + exp( curCoor.x / gamma);
            x_neg[i] = exp(-inputCoor.x / gamma) + exp(-curCoor.x / gamma);
            y_pos[i] = exp( inputCoor.y / gamma) + exp( curCoor.y / gamma);
            y_neg[i] = exp(-inputCoor.y / gamma) + exp(-curCoor.y / gamma);
            loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
            i++;
            // output net (only for the ff output to IO, to avoid double calculation)
            const vector<NextStage>& nextStage = cur_ff->getNextStage();
            if(nextStage.size()){
                // cout << nextStage.size() << endl;
                for(auto& next_p : nextStage){
                    std::string outputInstanceName;
                    if(next_p.outputGate)
                        outputInstanceName = next_p.outputGate->getInstanceName();
                    else // shift register
                        outputInstanceName = next_p.ff->getInstanceName();
                    std::string outputPinName = next_p.pinName;
                    curCoor = cur_ff->physicalFF->getNewCoor() + cur_ff->physicalFF->getPinCoor("Q" + cur_ff->getPhysicalPinName());
                    Coor outputCoor;
                    if(mgr.IO_Map.count(outputInstanceName)){
                        outputCoor = mgr.IO_Map[outputInstanceName].getCoor();
                    }
                    else if(mgr.Gate_Map.count(outputInstanceName)){
                        outputCoor = mgr.Gate_Map[outputInstanceName]->getCoor() + mgr.Gate_Map[outputInstanceName]->getPinCoor(outputPinName);
                    }
                    else{
                        FF* outputFF = mgr.preprocessor->getFFList()[outputInstanceName];
                        outputCoor = outputFF->physicalFF->getNewCoor() + outputFF->physicalFF->getPinCoor("D" + outputFF->getPhysicalPinName());
                    }
                    x_pos[i] = exp( outputCoor.x / gamma) + exp( curCoor.x / gamma);
                    x_neg[i] = exp(-outputCoor.x / gamma) + exp(-curCoor.x / gamma);
                    y_pos[i] = exp( outputCoor.y / gamma) + exp( curCoor.y / gamma);
                    y_neg[i] = exp(-outputCoor.y / gamma) + exp(-curCoor.y / gamma);
                    loss += log(x_pos[i]) + log(x_neg[i]) + log(y_pos[i]) + log(y_neg[i]);
                    i++;
                }
            }
        }
    }
    return loss;
}

vector<Coor>& postBankingObjFunction::backward(int step, bool onlyNegative){
    for(size_t i=0;i<grad_.size();i++){
        grad_[i].x = 0;
        grad_[i].y = 0;
    }
    int i=0;
    int idx=0;
    for(auto& FF_m : FF_list){
        FF* MBFF = FF_m.second;
        size_t bit = MBFF->getCell()->getBits();

        // weight of net
        size_t weightSize = bit;
        for(auto& clusterFF : MBFF->getClusterFF())
            weightSize += clusterFF->getNextStage().size();
        vector<double> weight(weightSize);
        getWeight(MBFF, weight);
        size_t curWeight = 0;
        for(size_t j=0;j<bit;j++){
            FF* cur_ff = MBFF->getClusterFF()[j];
            // net of D pin
            Coor curCoor = cur_ff->physicalFF->getNewCoor() + cur_ff->physicalFF->getPinCoor("D" + cur_ff->getPhysicalPinName());;
            grad_[idx].x += weight[curWeight] * ((exp(curCoor.x / gamma) / x_pos[i]) - (exp(-curCoor.x / gamma) / x_neg[i]));
            grad_[idx].y += weight[curWeight] * ((exp(curCoor.y / gamma) / y_pos[i]) - (exp(-curCoor.y / gamma) / y_neg[i]));   
            i++;
            curWeight++;
            // net of Q pin
            curCoor = cur_ff->physicalFF->getNewCoor() + cur_ff->physicalFF->getPinCoor("Q" + cur_ff->getPhysicalPinName());
            for(size_t k=0;k<cur_ff->getNextStage().size();k++){
                grad_[idx].x += weight[curWeight] * ((exp(curCoor.x / gamma) / x_pos[i]) - (exp(-curCoor.x / gamma) / x_neg[i]));
                grad_[idx].y += weight[curWeight] * ((exp(curCoor.y / gamma) / y_pos[i]) - (exp(-curCoor.y / gamma) / y_neg[i])); 
                i++;
                curWeight++;
            }
        }
        idx++;
    }
    return grad_;
}

void postBankingObjFunction::getWeight(FF* MBFF, vector<double>& weight){
    double sum = 0.0000001;
    // weigt by slack
    for(auto cur_ff : MBFF->getClusterFF()){
        double D_slack = cur_ff->physicalFF->getTimingSlack("D" + cur_ff->getPhysicalPinName());
        const vector<NextStage>& nextStage = cur_ff->getNextStage();
        // get total
        if(D_slack < 0)
            sum += D_slack;
        for(size_t j=0;j<nextStage.size();j++){
            double slack = nextStage[j].ff->physicalFF->getTimingSlack("D" + nextStage[j].ff->getPhysicalPinName());
            if(slack < 0)
                sum += slack;
        }
    }
    // get weight
    size_t curWeight = 0;
    for(auto cur_ff : MBFF->getClusterFF()){
        double D_slack = cur_ff->physicalFF->getTimingSlack("D" + cur_ff->getPhysicalPinName());
        const vector<NextStage>& nextStage = cur_ff->getNextStage();
        // get total
        if(D_slack < 0)
            weight[curWeight] = D_slack / sum;
        else
            weight[curWeight] = 0;
        curWeight++;
        for(size_t j=0;j<nextStage.size();j++){
            double slack = nextStage[j].ff->physicalFF->getTimingSlack("D" + nextStage[j].ff->getPhysicalPinName());
            if(slack < 0)
                weight[curWeight] = slack / sum;
            else
                weight[curWeight] = 0;
            curWeight++;
        }
    }
}

