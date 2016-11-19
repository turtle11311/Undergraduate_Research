#include "ThresholdNetwork.hpp"
#include <iostream>
using std::cout;
using std::endl;

Gate* ThresholdNetwork::accessGateByName(const char *const name)
{
    GateDict::iterator gateIt = gatePool.find(name);
    Gate* nowGate = (gateIt != gatePool.end()) ? gateIt->second : new Gate(name);
    gatePool[name] = nowGate;
    return nowGate;
}

void ThresholdNetwork::findCEVs(){
    std::vector<int> initialVec;
    for (auto& gate : gatePool){
            if ( gate.second->fan_in.size() == 0 ) continue;
            initialVec.resize(gate.second->fan_in.size());
            int uncheckedSum = 0;
            for ( int i = 0 ; i < initialVec.size() ; ++ i ){
                initialVec[i] = 0;
                uncheckedSum += std::get<1>(gate.second->fan_in[i]);
            }
            gate.second->onsetCriticalEffectVector(initialVec, 0, 0, uncheckedSum);
            //gate.second->_Debug_Gate_Information();
            //cout << "Onset table size: " << gate.second->onsetTable.size() << endl;
            //for ( int i = 0 ; i < gate.second->onsetTable.size() ; ++i ){
            //    cout << "【 ";
            //    for ( int j = 0 ; j < gate.second->onsetTable[i].size() ; j ++ )
            //    cout << gate.second->onsetTable[i][j] << " ";
            //    cout << " 】, ";
            //}
            //cout << endl;
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
    for ( auto gate : gatePool ){
        gate.second->_Debug_Gate_Information();
        if ( gate.second->fan_in.size() == 0 ) continue;
        cout << "Onset table size: " << gate.second->onsetTable.size() << endl;
        for ( int i = 0 ; i < gate.second->onsetTable.size() ; ++i ){
            cout << "【 ";
            for ( int j = 0 ; j < gate.second->onsetTable[i].size() ; j ++ )
            cout << gate.second->onsetTable[i][j] << " ";
            cout << " 】, ";
        }
        cout << endl;
    }
}

ThresholdNetwork::~ThresholdNetwork()
{
    for (auto& gate : gatePool) {
        delete gate.second;
    }
}
