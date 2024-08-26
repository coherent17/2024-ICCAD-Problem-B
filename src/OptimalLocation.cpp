#include "OptimalLocation.h"

objFunction::objFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, std::unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs)
    : mgr(mgr), FF_list(FF_list), idx_map(idx_map), loss(0), gamma((mgr.die.getDieBorder().x - mgr.die.getDieOrigin().x) * 0.01),
    x_pos(totalFF), x_neg(totalFF), 
    y_pos(totalFF), y_neg(totalFF),
    FFs(FFs){
}

preprocessObjFunction::preprocessObjFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, std::unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs)
    : objFunction(mgr, FF_list, idx_map, totalFF, FFs){
    grad_ = std::vector<Coor>(FFs.size());
    for(size_t i=0;i<FFs.size();i++){
        size_t size = FFs[i]->getNextStage().size() + 1;
        x_pos[i] = std::vector<double>(size);
        x_neg[i] = std::vector<double>(size);
        y_pos[i] = std::vector<double>(size);
        y_neg[i] = std::vector<double>(size);
    }
}

preprocessObjFunction::~preprocessObjFunction(){

}

/**
 * @brief build log sum exp hpwl approximate model
 * 
 * @return double the loss of the log sum exp model
 */
double preprocessObjFunction::forward(){
    loss = 0;
    //for(auto& FF_m : FF_list){
    #pragma omp parallel for num_threads(MAX_THREADS)
    for(size_t i=0;i<FFs.size();i++){
        FF* cur_ff = FFs[i];
        size_t net=0;
        Coor curCoor = cur_ff->getOriginalD();
        // input net
        if(cur_ff->getInputInstances().size() >= 1){
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
            x_pos[i][net] = exp( inputCoor.x / gamma) + exp( curCoor.x / gamma);
            x_neg[i][net] = exp(-inputCoor.x / gamma) + exp(-curCoor.x / gamma);
            y_pos[i][net] = exp( inputCoor.y / gamma) + exp( curCoor.y / gamma);
            y_neg[i][net] = exp(-inputCoor.y / gamma) + exp(-curCoor.y / gamma);
            loss += log(x_pos[i][net]) + log(x_neg[i][net]) + log(y_pos[i][net]) + log(y_neg[i][net]);
            net++;
        }
        else{
            x_pos[i][net] = 0;
            x_neg[i][net] = 0;
            y_pos[i][net] = 0;
            y_neg[i][net] = 0;
            loss += 0;
            net++;  
        }
        // output net
        const std::vector<NextStage>& nextStage = cur_ff->getNextStage();
        if(nextStage.size()){
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
                x_pos[i][net] = exp( outputCoor.x / gamma) + exp( curCoor.x / gamma);
                x_neg[i][net] = exp(-outputCoor.x / gamma) + exp(-curCoor.x / gamma);
                y_pos[i][net] = exp( outputCoor.y / gamma) + exp( curCoor.y / gamma);
                y_neg[i][net] = exp(-outputCoor.y / gamma) + exp(-curCoor.y / gamma);
                loss += log(x_pos[i][net]) + log(x_neg[i][net]) + log(y_pos[i][net]) + log(y_neg[i][net]);
                net++;
            }
        }
    }
    return loss;
}

/**
 * @brief Do partial derivative to get the gradient from the log sum exp model
 * 
 * @param step What does step actually do in this function?? @chengc119
 * @param onlyNegative 
 * @return std::vector<Coor>& 
 */
vector<Coor>& preprocessObjFunction::backward(int step, bool onlyNegative){
    for(size_t i=0;i<grad_.size();i++){
        grad_[i].x = 0;
        grad_[i].y = 0;
    }
    #pragma omp parallel for num_threads(MAX_THREADS)
    for(size_t i=0;i<FFs.size();i++){
        FF* cur_ff = FFs[i];
        size_t net=0;
        // weigt by slack
        const std::vector<NextStage>& nextStage = cur_ff->getNextStage();
        std::vector<double> weight(1 + nextStage.size());
        getWeight(cur_ff, weight);

        // net of D pin
        Coor curCoor = cur_ff->getOriginalD();
        if(cur_ff->getInputInstances().size() >= 1){
            grad_[i].x += weight[0] * ((exp(curCoor.x / gamma) / x_pos[i][net]) - (exp(-curCoor.x / gamma) / x_neg[i][net]));
            grad_[i].y += weight[0] * ((exp(curCoor.y / gamma) / y_pos[i][net]) - (exp(-curCoor.y / gamma) / y_neg[i][net]));  
        } 
        net++;
        // net of Q pin
        curCoor = cur_ff->getOriginalQ();
        for(size_t j=1;j<weight.size();j++){
            grad_[i].x += weight[j] * ((exp(curCoor.x / gamma) / x_pos[i][net]) - (exp(-curCoor.x / gamma) / x_neg[i][net]));
            grad_[i].y += weight[j] * ((exp(curCoor.y / gamma) / y_pos[i][net]) - (exp(-curCoor.y / gamma) / y_neg[i][net])); 
            net++;
        }
    }
    return grad_;
}

/**
 * @brief set the weight of the input pin and the output pin of the cur_ff
 * 
 * @param cur_ff the ff to calculate the input/output pin
 * @param weight the reference of the weight vectpr to set
 */
void preprocessObjFunction::getWeight(FF* cur_ff, std::vector<double>& weight){
    // weigt by slack
    double D_slack = cur_ff->getTimingSlack("D");
    const std::vector<NextStage>& nextStage = cur_ff->getNextStage();
    double sum = 0.0000001;
    // get total
    bool hasNegative = false;
    if(D_slack < 0){
        sum += D_slack;
        hasNegative = true;
    }
    for(size_t j=1;j<weight.size();j++){
        if(nextStage[j-1].ff->getTimingSlack("D") < 0){
            sum += nextStage[j-1].ff->getTimingSlack("D");
            hasNegative = true;
        }
    }
    // get weight
    // Only set the negative dslack pin weight, if there exist the negative dslack in fanin or fanout
    if(hasNegative){
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
    else{
        for(size_t i=0;i<weight.size();i++)
            weight[i] = 0;
    }
}

postBankingObjFunction::postBankingObjFunction(Manager&mgr, std::unordered_map<std::string, FF*>& FF_list, std::unordered_map<string, int>& idx_map, int totalFF, std::vector<FF*>& FFs)
    : objFunction(mgr, FF_list, idx_map, totalFF, FFs){
    grad_ = std::vector<Coor>(FFs.size());
    for(size_t i=0;i<FFs.size();i++){
        size_t size = 0;
        for(const auto& ff : FFs[i]->getClusterFF()){
            size += 1 + ff->getNextStage().size();
        }
        x_pos[i] = std::vector<double>(size);
        x_neg[i] = std::vector<double>(size);
        y_pos[i] = std::vector<double>(size);
        y_neg[i] = std::vector<double>(size);
    }
}

postBankingObjFunction::~postBankingObjFunction(){

}

double postBankingObjFunction::forward(){
    loss = 0;
    // int i=0;
    // for(auto& FF_m : FF_list){
    #pragma omp parallel for num_threads(MAX_THREADS)
    for(size_t i=0;i<FFs.size();i++){
        FF* MBFF = FFs[i];
        size_t net = 0;
        size_t bit = MBFF->getCell()->getBits();
        for(size_t j=0;j<bit;j++){
            FF* cur_ff = MBFF->getClusterFF()[j];
            // input net
            Coor curCoor(0, 0);
            if(cur_ff->getInputInstances().size() >= 1){
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
                curCoor = cur_ff->physicalFF->getNewCoor() + cur_ff->physicalFF->getPinCoor("D" + cur_ff->getPhysicalPinName());
                x_pos[i][net] = exp( inputCoor.x / gamma) + exp( curCoor.x / gamma);
                x_neg[i][net] = exp(-inputCoor.x / gamma) + exp(-curCoor.x / gamma);
                y_pos[i][net] = exp( inputCoor.y / gamma) + exp( curCoor.y / gamma);
                y_neg[i][net] = exp(-inputCoor.y / gamma) + exp(-curCoor.y / gamma);
                loss += log(x_pos[i][net]) + log(x_neg[i][net]) + log(y_pos[i][net]) + log(y_neg[i][net]);
                net++;
            }
            else{
                x_pos[i][net] = 0;
                x_neg[i][net] = 0;
                y_pos[i][net] = 0;
                y_neg[i][net] = 0;
                loss += 0;
                net++;  
            }
            // output net (only for the ff output to IO, to avoid double calculation)
            const std::vector<NextStage>& nextStage = cur_ff->getNextStage();
            if(nextStage.size()){
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
                    x_pos[i][net] = exp( outputCoor.x / gamma) + exp( curCoor.x / gamma);
                    x_neg[i][net] = exp(-outputCoor.x / gamma) + exp(-curCoor.x / gamma);
                    y_pos[i][net] = exp( outputCoor.y / gamma) + exp( curCoor.y / gamma);
                    y_neg[i][net] = exp(-outputCoor.y / gamma) + exp(-curCoor.y / gamma);
                    loss += log(x_pos[i][net]) + log(x_neg[i][net]) + log(y_pos[i][net]) + log(y_neg[i][net]);
                    net++;
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
    #pragma omp parallel for num_threads(MAX_THREADS)
    for(size_t i=0;i<FFs.size();i++){
        FF* MBFF = FFs[i];
        size_t net = 0;
        size_t bit = MBFF->getCell()->getBits();

        // weight of net
        size_t weightSize = bit;
        for(auto& clusterFF : MBFF->getClusterFF())
            weightSize += clusterFF->getNextStage().size();
        std::vector<double> weight(weightSize);
        getWeight(MBFF, weight);
        size_t curWeight = 0;
        for(size_t j=0;j<bit;j++){
            FF* cur_ff = MBFF->getClusterFF()[j];
            // net of D pin
            Coor curCoor = cur_ff->physicalFF->getNewCoor() + cur_ff->physicalFF->getPinCoor("D" + cur_ff->getPhysicalPinName());
            if(cur_ff->getInputInstances().size() >= 1){
                grad_[i].x += weight[curWeight] * ((exp(curCoor.x / gamma) / x_pos[i][net]) - (exp(-curCoor.x / gamma) / x_neg[i][net]));
                grad_[i].y += weight[curWeight] * ((exp(curCoor.y / gamma) / y_pos[i][net]) - (exp(-curCoor.y / gamma) / y_neg[i][net]));   
            }
            net++;
            curWeight++;
            // net of Q pin
            curCoor = cur_ff->physicalFF->getNewCoor() + cur_ff->physicalFF->getPinCoor("Q" + cur_ff->getPhysicalPinName());
            for(size_t k=0;k<cur_ff->getNextStage().size();k++){
                grad_[i].x += weight[curWeight] * ((exp(curCoor.x / gamma) / x_pos[i][net]) - (exp(-curCoor.x / gamma) / x_neg[i][net]));
                grad_[i].y += weight[curWeight] * ((exp(curCoor.y / gamma) / y_pos[i][net]) - (exp(-curCoor.y / gamma) / y_neg[i][net])); 
                net++;
                curWeight++;
            }
        }
    }
    return grad_;
}

void postBankingObjFunction::getWeight(FF* MBFF, std::vector<double>& weight){
    double sum = 0.0000001;
    bool hasNegative = false;
    // weigt by slack
    for(auto cur_ff : MBFF->getClusterFF()){
        double D_slack = cur_ff->physicalFF->getTimingSlack("D" + cur_ff->getPhysicalPinName());
        const std::vector<NextStage>& nextStage = cur_ff->getNextStage();
        // get total
        if(D_slack < 0){
            sum += D_slack;
            hasNegative = true;
        }
        for(size_t j=0;j<nextStage.size();j++){
            double slack = nextStage[j].ff->physicalFF->getTimingSlack("D" + nextStage[j].ff->getPhysicalPinName());
            if(slack < 0){
                sum += slack;
                hasNegative = true;
            }
        }
    }
    // get weight
    size_t curWeight = 0;
    for(auto cur_ff : MBFF->getClusterFF()){
        double D_slack = cur_ff->physicalFF->getTimingSlack("D" + cur_ff->getPhysicalPinName());
        const std::vector<NextStage>& nextStage = cur_ff->getNextStage();
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
    if(!hasNegative){
        for(size_t i=0;i<weight.size();i++)
            weight[i] = 0;
    }
}

Gradient::Gradient( Manager &mgr,
                    std::unordered_map<std::string, FF*>& FF_list,
                    objFunction &obj,
                    const double &alpha,
                    std::unordered_map<string, int>& idx_map,
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
}

Gradient::~Gradient(){

}

void Gradient::Initialize(double kAlpha) {
    step_ = 0;
    alpha_ = kAlpha;
}

void Gradient::Step(bool onlyNegative) {
    // Compute the gradient direction
    obj_.forward();
    obj_.backward(step_, onlyNegative);
    // Compute the Polak-Ribiere coefficient and conjugate directions

    if (step_ == 0) {
        // For the first step, we will set beta = 0 and d_0 = -g_0
        for (size_t i = 0; i < kNumModule; ++i) {
            dir[i].x = -obj_.grad().at(i).x;
            dir[i].y = -obj_.grad().at(i).y;
        }
    } else {
        // For the remaining steps, we will calculate the Polak-Ribiere coefficient and
        // conjugate directions normally
        double t1 = 0.;  // Store the numerator of beta
        double t2 = 0.;  // Store the denominator of beta
        #pragma omp parallel for reduction(+:t1, t2)
        for (size_t i = 0; i < kNumModule; ++i) {
            Coor g = obj_.grad().at(i);
            Coor t3;
            t3.x = g.x * (g.x - grad_prev_.at(i).x);
            t3.y = g.y * (g.y - grad_prev_.at(i).y);
            t1 += t3.x + t3.y;
            t2 += std::abs(g.x) + std::abs(g.y);
        }
        double beta = t1 / (t2 * t2 + 0.00001); // Polak-Ribiere coefficient
        #pragma omp parallel for num_threads(MAX_THREADS)
        for (size_t i = 0; i < kNumModule; ++i) {
            dir[i].x = -obj_.grad().at(i).x + beta * dir_prev_.at(i).x;
            dir[i].y = -obj_.grad().at(i).y + beta * dir_prev_.at(i).y;
        }
    }
    // Update the solution
    #pragma omp parallel for num_threads(MAX_THREADS)
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
