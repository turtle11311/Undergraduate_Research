#include "ThresholdNetwork.hpp"
#include <iostream>
#include <numeric>
using std::cout;
using std::endl;

Gate* ThresholdNetwork::accessGateByName(const char *const name)
{
    GateDict::iterator gateIt = gatePool.find(name);
    Gate* nowGate = (gateIt != gatePool.end()) ? gateIt->second : new Gate(name);
    gatePool[name] = nowGate;
    return nowGate;
}

void ThresholdNetwork::findCEVs()
{
    std::vector<int> initialVec;
    for (auto& gate : gatePool) {
        if (gate.second->fan_in.size() == 0) continue;
        initialVec.assign(gate.second->fan_in.size(), 0);
        int uncheckedSum =
            std::accumulate(gate.second->fan_in.begin(), gate.second->fan_in.end(), 0,
            [](int sum, const ThresholdInput &thg) {
                return std::get<1>(thg) + sum;
            }
        );
        gate.second->onsetCriticalEffectVector(initialVec, 0, 0, uncheckedSum);
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
