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
        initialVec.assign(gate.second->fan_in.size(), 1);
        gate.second->offsetCriticalEffectVector(initialVec, 0, 0, uncheckedSum);
    }
}

void ThresholdNetwork::findAllDominator()
{
    /*!
     * \fn dominator(Gate* gate)
     * \brief find this gate dominators
     * \param gate the gate, that you want to find its dominator
     */
    std::function<std::set<Gate*>*(Gate*)> dominator = [=, &dominator](Gate* gate) -> std::set<Gate*>*
    {
        if (!gate->dominators.empty() || gate->type == PO) {
            return &gate->dominators;
        }
        std::set<Gate*> intersection(*dominator(gate->fan_out.front()));
        std::set<Gate*> tmpSet;
        for (auto it = ++gate->fan_out.begin(); it != gate->fan_out.end(); ++it) {
            auto itDominator = dominator(*it);
            tmpSet.clear();
            std::set_intersection(itDominator->begin(), itDominator->end(),
            intersection.begin(), intersection.end(),
            std::inserter(tmpSet, tmpSet.begin()));
            intersection = tmpSet;
        }
        intersection.insert(gate);
        gate->dominators = intersection;
        return &gate->dominators;
    };

    for (Gate* PI : start.fan_out) {
        dominator(PI);
    }
}

void ThresholdNetwork::foreachGateAttr()
{
    for (auto &gate : start.fan_out) {
        gate->evalFanoutCone();
    }
    for (auto &gate : gatePool) {
        gate.second->evalSideInput();
        cout << gate.first << " SideInput: " << endl;
        for (Gate* side : gate.second->sideInputs) {
            cout << side->name << ", ";
        }
        cout << endl;
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
    for ( auto& gate : gatePool ){
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
        cout << "Offset table size: " << gate.second->offsetTable.size() << endl;
        for ( int i = 0 ; i < gate.second->offsetTable.size() ; ++i ){
            cout << "【 ";
            for ( int j = 0 ; j < gate.second->offsetTable[i].size() ; j ++ )
            cout << gate.second->offsetTable[i][j] << " ";
            cout << " 】, ";
        }
        cout << endl;
    }
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
            end.fan_in.push_back(ThresholdInput{gate.second,1,0});
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
