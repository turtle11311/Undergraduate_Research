#include "ThresholdNetwork.hpp"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <functional>

using std::cout;
using std::endl;

Gate* ThresholdNetwork::accessGateByName(const char *const name)
{
    GateDict::iterator gateIt = gatePool.find(name);
    Gate* nowGate = (gateIt != gatePool.end()) ? gateIt->second : new Gate(name);
    gatePool[name] = nowGate;
    return nowGate;
}

void ThresholdNetwork::foreachGateAttr()
{
    for (Gate* gate : start.fan_out) {
        gate->evalDominators();
        gate->evalFanoutCone();
    }
    for (auto &gate : gatePool) {
        gate.second->evalSideInput();
        gate.second->evalCriticalEffectVectors();
        gate.second->checkContollingValueState(1);
        gate.second->checkContollingValueState(0);
    }
}

void ThresholdNetwork::_Debug_Wiring()
{
    for (auto& gate : gatePool) {
        std::cout << "Name: " << gate.first << "-> " << gate.second->thresholdVal << std::endl;
        std::cout << "FANIN: " << std::endl;
        for (auto& fanin : gate.second->fan_in) {
            std::cout << std::get<0>(fanin)->name << ": "
                      << std::get<1>(fanin) << std::endl;
        }
        std::cout << "FANOUT: " << std::endl;
        for (auto& fanout : gate.second->fan_out) {
            std::cout << fanout->name << ", ";
        }
        std::cout << std::endl << std::endl;
    }
}

void ThresholdNetwork::_Debug_Onset_Critical_Effect_Vector(){
    cout << "!!!!!!!!!!\n";
    for ( auto& gate : gatePool ){
        if ( gate.second->type == PI ) continue;

        gate.second->_Debug_Gate_Information();
        if ( gate.second->fan_in.size() == 0 ) continue;
        cout << "Onset table size: " << gate.second->onsetTable.size() << endl;
        for ( unsigned int i = 0 ; i < gate.second->onsetTable.size() ; ++i ){
            cout << "【 ";
            for ( unsigned int j = 0 ; j < gate.second->onsetTable[i].size() ; j ++ )
            cout << gate.second->onsetTable[i][j] << " ";
            cout << " 】, ";
        }
        cout << endl;
        cout << "Offset table size: " << gate.second->offsetTable.size() << endl;
        for ( unsigned int i = 0 ; i < gate.second->offsetTable.size() ; ++i ){
            cout << "【 ";
            for ( unsigned int j = 0 ; j < gate.second->offsetTable[i].size() ; j ++ )
            cout << gate.second->offsetTable[i][j] << " ";
            cout << " 】, ";
        }
        cout << endl;
    }
}

void ThresholdNetwork::_Debug_Controlling_Value(){

    cout << "func: _Debug_Controlling_Value\n";
    for ( auto& gate : gatePool ){
        if ( gate.second->type == PI ) continue;

        cout << "Gate name: " << gate.first << endl;
        cout << "Gate controlling value state: ";
        for ( auto& fanin : gate.second->fan_in ){
            cout << std::get<3>(fanin) << " ";
        }
        cout << endl << endl;
    }
}

void ThresholdNetwork::_Debug_Check_The_Sum_Of_The_Number_Of_Side_Inputs(){

    int maximum = 0;
    for ( auto& gate : gatePool ){
        if ( gate.second->type == PI ) continue;
        //cout << "Gate name: " << gate.first << endl;
        int initialMas = 0;
        cout << gate.second->sideInputs.size() << " : ";
        for ( Gate* si : gate.second->sideInputs ){
            initialMas += si->sideInputControllingValCount;
        }
        if ( maximum < initialMas )
            maximum = initialMas;
        cout << initialMas << endl;
    }
    cout << maximum << endl;
}

void ThresholdNetwork::gateClassify(){
    for ( auto& gate : gatePool ){
        if ( gate.second->fan_in.size() == 0 && gate.second->fan_out.size() == 0 )
            gate.second->type = Constant;
        else if ( gate.second->fan_in.size() == 0 ){
            gate.second->type = PI;
            start.fan_out.push_back(gate.second);
        }
        else if ( gate.second->fan_out.size() == 0 ){
            gate.second->type = PO;
            end.fan_in.push_back(ThresholdInput{gate.second,1,0,-1});
        }
        else
            gate.second->type = Internal;
    }
}

ThresholdNetwork::~ThresholdNetwork()
{
    for (auto& gate : gatePool) {
        delete gate.second;
    }
}
