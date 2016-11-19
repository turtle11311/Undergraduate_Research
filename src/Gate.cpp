#include "Gate.hpp"
#include <iostream>
using std::cout;
using std::endl;

Gate::Gate(const char* const name)
    : name(name)
{}

void Gate::addInput(Gate *input, int thresholdVal, bool phase)
{
    this->fan_in.push_back(ThresholdInput{input, thresholdVal, phase});
    input->fan_out.push_back(this);
}

void Gate::onsetCriticalEffectVector(std::vector<int> curPattern, int pos, int curWeightSum, int uncheckedSum) {
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

void Gate::offsetCriticalEffectVector(std::vector<int> curPattern, int pos, int curWeightSum, int uncheckedSum){
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

void Gate::_Debug_Gate_Information(){
    cout << "Name: " << name << endl;
    cout << "Threshold value: " << thresholdVal << endl;
    cout << "Fanins: " << endl;
    for ( int i = 0 ; i < fan_in.size() ; ++i ){
        cout << "【" << std::get<0>(fan_in[i])->name << " : "
                     << std::get<1>(fan_in[i])
             << "】, ";
    }
    cout << endl;
    cout << "Fanouts: " << endl;
    for ( int i = 0 ; i < fan_out.size() ; ++i ){
        cout << "【" << fan_out[i]->name << "】, ";
    }
    cout << endl;
}
