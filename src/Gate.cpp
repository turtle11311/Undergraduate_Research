#include "Gate.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <iterator>
using std::cout;
using std::endl;
using std::get;

Gate::Gate(const char* const name)
    : name(name), value(-1), onsetStage(0), offsetStage(0), determinedFaninCount(0), totalWeight(0), constTotalWeight(0)
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

std::vector<Gate*> Gate::backwardChecking(){
    if ( name == "v8" )
        cout << "got u2!!!!!!!!!!!!!\n";
    std::vector<Gate*> implyGateList;
    Gate temp = *this;
    temp.thresholdVal = thresholdVal;
    temp.fan_in.clear();
    for ( auto& fanin : fan_in ){
        if ( fanin.ptr->value != -1 ){
                temp.thresholdVal -= fanin.ptr->value;
        }
        temp.fan_in.push_back(fanin);
    }
    temp.onsetTable.clear();
    temp.offsetTable.clear();
    temp.evalCriticalEffectVectors();
    temp.checkContollingValueState();
    if ( name == "v8" ){
        cout << temp.fan_in.size() << endl;
        cout << onsetTable.size() << endl;
        cout << "############################\n";
        for ( int i = 0 ; i < temp.onsetTable.size() ; ++i ){
            cout << "\tosSize: " << onsetTable[i].size() << endl;
            for ( int j = 0 ; j < temp.onsetTable[i].size() ; ++j ){
                cout << "\t\tindex: " << j << endl;
                cout << onsetTable[i][j] << ", ";
            }
            cout << endl;
        }
        cout << offsetTable.size() << endl;
        cout << "############################\n";
        for ( int i = 0 ; i < temp.offsetTable.size() ; ++i ){
            for ( int j = 0 ; j < temp.offsetTable[i].size() ; ++j ){
                cout << offsetTable[i][j] << ", ";
            }
            cout << endl;
        }
        cout << "############################\n";
    }
    for ( auto& fanin : temp.fan_in ){
        if ( fanin.ptr->value == -1 ){
            if ( value == 1 ){
                if ( !fanin.inverter ){
                    if ( fanin.ctrlVal == 1 ){
                        fanin.ptr->value = 1;
                        implyGateList.push_back(fanin.ptr);
                    }
                }else {
                    if ( fanin.ctrlVal == 1 ){
                        fanin.ptr->value = 0;
                        implyGateList.push_back(fanin.ptr);
                    }
                }
            }
            else{
                if ( !fanin.inverter ){
                    if ( fanin.ctrlVal == 0 ){
                        fanin.ptr->value = 0;
                        implyGateList.push_back(fanin.ptr);
                    }
                }else {
                    if ( fanin.ctrlVal == 0 ){
                        fanin.ptr->value = 1;
                        implyGateList.push_back(fanin.ptr);
                    }
                }
            }
        }
    }
    return implyGateList;
}

int Gate::directEvalRes(){
    int weightSum = 0;
    for ( int i = 0 ; i < fan_in.size() ; ++i ){
        if ( !fan_in[i].inverter ){
            if ( fan_in[i].ptr->value != -1 ){
                weightSum += (fan_in[i].ptr->value == 1) ? fan_in[i].weight : 0;
                totalWeight -= fan_in[i].weight;
                if ( name == "v8" ){
                    cout << "===============\n";
                }
            }
        }
        else{
            if ( fan_in[i].ptr->value != -1 ){
                weightSum += (fan_in[i].ptr->value == 0) ? fan_in[i].weight : 0;
                totalWeight -= fan_in[i].weight;
                if ( name == "v8" ){
                    cout << "===============\n";
                }
            }
        }
        if ( weightSum  >= thresholdVal ){
            return 1;
        }
        else if ( totalWeight + weightSum < thresholdVal ){
            return 0;
        }
    }
    if ( name == "v8" ){
        cout << "..........................\n";
        cout << "weightSum: " << weightSum << " totalWeight: " << totalWeight <<endl;
    }
    return -1;
}

bool Gate::exhaustiveChecking(){
    for ( int i = 0 ; i < fan_in.size() ; ++i )
        if ( fan_in[i].ptr->value == -1 )
            return false;
    return true;
}

void Gate::refreshDeterminedFaninCount( bool subtract ){

    for ( Gate* fanout : fan_out )
        fanout->determinedFaninCount = ( subtract ) ? fanout->determinedFaninCount-1 :
                                                        fanout->determinedFaninCount+1;
}

std::set<Gate*>* Gate::evalDominators()
{
    if ( !dominators.empty()) {
        return &dominators;
    }
    if ( type == PO ){
        dominators.insert(this);
        return &dominators;
    }
    // cout << "\tCurrent: " << name << endl;
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

void Gate::_Debug_Fanout_Cone(){
    cout << name << "\'s fanoutCone: ";
    for ( Gate* fanoutConeGate : fanoutCone )
    cout << fanoutConeGate->name << ", ";
    cout << endl;
}

std::set<Gate*>* Gate::evalFanoutCone()
{
    if (!fanoutCone.empty() ) {
        return &fanoutCone;
    }
    if ( type == PO ){
        fanoutCone.insert(this);
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

void Gate::_Debug_Side_Inputs(){
    cout << name << "\'s sideInputs: ";
    for ( Gate* si : sideInputs ){
        cout << si->name << ", ";
    }
    cout <<endl;
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
    sideInputs.erase(this);
}

void Gate::checkContollingValueState(){
    // check onsetTable
    if ( onsetTable.size() == 1 ){
        for ( int pos = 0 ; pos < fan_in.size() ; ++pos ){
            if ( onsetTable[0][pos] == 1 ){
                fan_in[pos].ctrlVal = 1;
            }
        }
    }
    for ( int pos = 0 ; pos < fan_in.size() ; ++pos  ){
        bool flag = true;
        for ( int i = 0; i < onsetTable.size() ; ++i ){
            if ( onsetTable[i][pos] == 0 ){
                flag = false;
                break;
            }
        }
        if ( flag ) fan_in[pos].ctrlVal = 1;
        else break;
    }
    // check offsetTable
    if ( offsetTable.size() == 1 ){
        for ( int pos = 0 ; pos < fan_in.size() ; ++pos ){
            if ( offsetTable[0][pos] == 0 )
                fan_in[pos].ctrlVal = 0;
        }
    }
    for ( int pos = 0 ; pos < fan_in.size() ; ++pos  ){
        bool flag = true;
        for ( int i = 0; i < offsetTable.size() ; ++i ){
            if ( offsetTable[i][pos] == 1 ){
                flag = false;
                break;
            }
        }
        if ( flag ) fan_in[pos].ctrlVal = 0;
        else break;
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

int Gate::evalIndirectImnplicationList(int mode){

    if ( mode ){
        onsetStage = 1;
        for ( int i = 0 ; i < onsetTable.size() ; ++i ){
            if ( onsetTable[i][onsetStage-1] == 0 )
                ++onsetStage;
            if ( onsetStage > indirectLevelConstraint )
                break;
        }
    }
    else {
        offsetStage = 1;
        for ( int i = 0 ; i < offsetTable.size() ; ++i ){
            if ( offsetTable[i][offsetStage-1] == 1 )
                ++offsetStage;
            if ( offsetStage > indirectLevelConstraint )
                break;
        }
    }
    return mode ? onsetStage : offsetStage;
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
