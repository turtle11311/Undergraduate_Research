#include "ThresholdNetwork.hpp"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <functional>
#include <list>

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
            std::cout << fanin.ptr->name << ": "
                      << fanin.weight << std::endl;
        }
        std::cout << "FANOUT: " << std::endl;
        for (auto& fanout : gate.second->fan_out) {
            std::cout << fanout->name << ", ";
        }
        std::cout << std::endl << std::endl;
    }
}


void ThresholdNetwork::evalMandatoryAssignments()
{
    for (auto gate : gatePool) {
        if (!gate.second->sideInputs.empty())
            targetGateList.push_back(gate.second);
    }
    std::list<ImplacationGate> queue;
    for (Gate* target : targetGateList) {
        // Stuck at 0
        target->value = 1;
        queue.push_back(ImplacationGate({ target, FORWARD}));
        queue.push_back(ImplacationGate({ target, BACKWARD}));
        while (!queue.empty()) {
            auto nowGate = queue.front();
            switch (nowGate.action) {
            case FORWARD:
                for (Gate* fanout : nowGate.ptr->fan_out) {
                    if (fanout->value != -1) {
                        queue.push_back(ImplacationGate({fanout, BACKWARD}));
                        continue;
                    }
                    // Have three states: no ctrlVal, same ctrlVal and not same ctrlVal
                    if (fanout->getInput(nowGate.ptr).ctrlVal == -1) {
                        // DO NOTHING
                    } else if (fanout->getInput(nowGate.ptr).ctrlVal == nowGate.ptr->value) {
                        fanout->value = nowGate.ptr->value;
                        queue.push_back(ImplacationGate({fanout, FORWARD}));
                    } else {
                        /* Refresh fanout gate CEV */
                    }
                }
                break;
            case BACKWARD:
                for (auto &fanin : nowGate.ptr->fan_in) {
                    if (nowGate.ptr->value == 0) {
                        if (fanin.ctrlVal != -1) {
                            if (fanin.ptr->value == -1) {
                                fanin.ptr->value = 0;
                                queue.push_back(ImplacationGate({fanin.ptr, BACKWARD}));
                                queue.push_back(ImplacationGate({fanin.ptr, FORWARD}));
                            }
                            else {
                                if (fanin.ptr->value == 1) {
                                    queue.clear();
                                    // add flag
                                }
                            }
                        }
                    }
                    else if (nowGate.ptr->value == 1) {
                        //  indirect
                    }
                }
                break;
            default:
                break;
            }
            queue.pop_front();
        }
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
            cout << fanin.ctrlVal << " ";
        }
        cout << endl << endl;
    }
}

void ThresholdNetwork::_Debug_Check_The_Sum_Of_The_Number_Of_Side_Inputs()
{
    cout << "Information:" << endl;
    cout << "Gate number:" << gatePool.size() - start.fan_out.size() - end.fan_in.size() << endl;
    cout << "TargetGate number: " << targetGateList.size() << endl;
    cout << endl << endl;
    int max = 0;
    for (auto gate: targetGateList) {
        cout << "\tGatename: " << gate->name << endl;
        cout << "\tSideinput number: " << gate->sideInputs.size() << endl;
        cout << "\tControlling value number: " << gate->sideInputControllingValCount << endl;
        max = max < gate->sideInputControllingValCount ? gate->sideInputControllingValCount : max;
    }
    cout << "max contolling value number: " << max << endl;
    cout << endl << endl << endl;
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
            end.fan_in.push_back(ThresholdInput({gate.second, 1, 0, -1}));
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
