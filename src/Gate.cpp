#include "Gate.hpp"
#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;
using std::get;

Gate::Gate(const char* const name)
    : name(name)
{ sideInputControllingValCount = 0;}

void Gate::addInput(Gate *input, int thresholdVal, bool phase)
{
    this->fan_in.push_back(ThresholdInput{input, thresholdVal, phase, -1});
    input->fan_out.push_back(this);
}

void Gate::onsetCriticalEffectVector(std::vector<int> curPattern, unsigned int pos, int curWeightSum, int uncheckedSum) {
    if ( pos == curPattern.size() ) return;
    if ( uncheckedSum < thresholdVal - curWeightSum ) return;
    int checkBitWeight = std::get<1>(fan_in[pos]);
    onsetCriticalEffectVector( curPattern, pos + 1, curWeightSum, uncheckedSum - checkBitWeight );
    curWeightSum += checkBitWeight;
    curPattern[pos] = 1;
    if ( curWeightSum >= thresholdVal )
        onsetTable.push_back(curPattern);
    else
        onsetCriticalEffectVector( curPattern, pos + 1, curWeightSum, uncheckedSum - checkBitWeight );
}

void Gate::offsetCriticalEffectVector(std::vector<int> curPattern, unsigned int pos, int curWeightSum, int uncheckedSum){
    if ( pos == curPattern.size() ) return;
    if ( curWeightSum >= thresholdVal ) return;
    int checkBitWeight = std::get<1>(fan_in[pos]);
    curWeightSum += checkBitWeight;
    offsetCriticalEffectVector( curPattern, pos + 1, curWeightSum, uncheckedSum - checkBitWeight );
    curPattern[pos] = 0;
    curWeightSum -= checkBitWeight;
    if ( uncheckedSum + curWeightSum - checkBitWeight < thresholdVal )
        offsetTable.push_back(curPattern);
    else
        offsetCriticalEffectVector( curPattern, pos + 1, curWeightSum, uncheckedSum - checkBitWeight );
}

void Gate::evalCriticalEffectVectors()
{
    std::vector<int> initialVec;
    if (fan_in.size() == 0) return;
    initialVec.assign(fan_in.size(), 0);
    int uncheckedSum =
        std::accumulate(fan_in.begin(), fan_in.end(), 0,
            [](int sum, const ThresholdInput &thg) {
                    return std::get<1>(thg) + sum;
            }
        );
    onsetCriticalEffectVector(initialVec, 0, 0, uncheckedSum);
    initialVec.assign(fan_in.size(), 1);
    offsetCriticalEffectVector(initialVec, 0, 0, uncheckedSum);
}

std::set<Gate*>* Gate::evalDominators()
{
    if (!dominators.empty() || type == PO) {
        return &dominators;
    }
    std::set<Gate*> intersection(*fan_out.front()->evalDominators());
    std::set<Gate*> tmpSet;
    for (auto it = ++fan_out.begin(); it != fan_out.end(); ++it) {
        auto itDominator = (*it)->evalDominators();
        tmpSet.clear();
        std::set_intersection(itDominator->begin(), itDominator->end(),
        intersection.begin(), intersection.end(),
        std::inserter(tmpSet, tmpSet.begin()));
        intersection = tmpSet;
    }
    intersection.insert(this);
    dominators = intersection;
    return &dominators;
}

std::set<Gate*>* Gate::evalFanoutCone()
{
    if (!fanoutCone.empty() || type == PO) {
        return &fanoutCone;
    }

    std::set<Gate*> _union(*fan_out.front()->evalFanoutCone());
    std::set<Gate*> tmpSet;
    for (auto it = ++fan_out.begin(); it != fan_out.end(); ++it) {
        auto itFanoutCone = (*it)->evalFanoutCone();
        tmpSet.clear();
        std::set_union(itFanoutCone->begin(), itFanoutCone->end(),
                       _union.begin(), _union.end(),
                       std::inserter(tmpSet, tmpSet.begin()));
        _union = tmpSet;
    }
    _union.insert(this);
    fanoutCone = _union;
    return &fanoutCone;
}

void Gate::evalSideInput()
{
    for (Gate* dominator : dominators) {
        for (auto &fanin : dominator->fan_in) {
            Gate* ptr = get<0>(fanin);
            if (fanoutCone.find(ptr) == fanoutCone.end()) {
                sideInputs.insert(ptr);
            }
        }
    }
    // purge its fan_in
    for (auto& gate : fan_in) {
        Gate *ptr = get<0>(gate);
        sideInputs.erase(ptr);
    }
}

void Gate::checkContollingValueState( int mode ){
    auto& table = mode ? onsetTable : offsetTable;
    for ( unsigned int i = 0 ; i < table.size() ; ++i ){
        int index = -1;
        for ( unsigned int j = 0 ; j < table[i].size() ; ++j ){
            if ( table[i][j] == mode ){
                if ( index == -1 ){
                    index = j;
                }
                else {
                    index = -1;
                    break;
                }
            }
        }
        if ( index != -1 ) {
            if ( std::get<3>(fan_in[index]) != -1 ) {
                std::get<3>(fan_in[index]) = 2;
            }
            else {
                std::get<3>(fan_in[index]) = mode;
            }
            ++sideInputControllingValCount;
        }
    }
}

void Gate::_Debug_Gate_Information(){
    cout << "Name: " << name << endl;
    cout << "Threshold value: " << thresholdVal << endl;
    cout << "Fanins: " << endl;
    for ( unsigned int i = 0 ; i < fan_in.size() ; ++i ){
        cout << "【" << std::get<0>(fan_in[i])->name << " : "
                     << std::get<1>(fan_in[i])
             << "】, ";
    }
    cout << endl;
    cout << "Fanouts: " << endl;
    for ( unsigned int i = 0 ; i < fan_out.size() ; ++i ){
        cout << "【" << fan_out[i]->name << "】, ";
    }
    cout << endl;
}
