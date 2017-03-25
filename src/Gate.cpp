#include "Gate.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <iterator>
using std::cout;
using std::endl;
using std::get;

Gate::Gate(const char* const name)
    : name(name), value(-1)
{ sideInputControllingValCount = 0;}

void Gate::addInput(Gate *input, int thresholdVal, bool phase)
{
    this->fan_in.push_back(ThresholdInput{input, thresholdVal, phase, -1});
    input->fan_out.push_back(this);
}

void Gate::onsetCriticalEffectVector(std::vector<int> curPattern, unsigned int pos, int curWeightSum, int uncheckedSum) {
    if ( pos == curPattern.size() ) return;
    if ( uncheckedSum < thresholdVal - curWeightSum ) return;
    int checkBitWeight = fan_in[pos].weight;
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
    int checkBitWeight = fan_in[pos].weight;
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
                    return thg.weight + sum;
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
            Gate* ptr = fanin.ptr;
            if (fanoutCone.find(ptr) == fanoutCone.end()) {
                sideInputs.insert(ptr);
            }
        }
    }
    // purge its fan_in
    for (auto& gate : fan_in) {
        sideInputs.erase(gate.ptr);
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
            if ( fan_in[index].ctrlVal != -1 ) {
                fan_in[index].ctrlVal = 2;
            }
            else {
                fan_in[index].ctrlVal = mode;
            }
            ++sideInputControllingValCount;
        }
    }
}

static const ThresholdInput nullGate({ nullptr, -1, -1, -1 });

const ThresholdInput& Gate::getInput(const Gate* target)
{
    for (const ThresholdInput& input : fan_in) {
        if (input.ptr == target) {
            return input;
        }
    }
    return nullGate;
}

void Gate::_Debug_Gate_Information(){
    cout << "Name: " << name << endl;
    cout << "Threshold value: " << thresholdVal << endl;
    cout << "Fanins: " << endl;
    for ( unsigned int i = 0 ; i < fan_in.size() ; ++i ){
        cout << "【" << fan_in[i].ptr->name << " : "
                     << fan_in[i].weight
             << "】, ";
    }
    cout << endl;
    cout << "Fanouts: " << endl;
    for ( unsigned int i = 0 ; i < fan_out.size() ; ++i ){
        cout << "【" << fan_out[i]->name << "】, ";
    }
    cout << endl;
}
