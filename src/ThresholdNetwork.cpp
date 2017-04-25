#include "ThresholdNetwork.hpp"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <functional>
#include <list>

using std::cout;
using std::endl;


ThresholdNetwork::ThresholdNetwork(){
}

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
        gate.second->checkContollingValueState();
    }
}

void ThresholdNetwork::_Debug_Wiring(){
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

std::set<GateWithValue> ThresholdNetwork::iterativeImplication( Gate* target){
    std::set<GateWithValue> MASet;
    MASet.insert(GateWithValue({target,target->value}));
    int pos = modifyList.size();
    cout << "Target: " << target->name << " | start."<< endl;
    cout << "Side number： " << target->sideInputs.size() << endl;
    bool hasMA = true;
    while (!queue.empty() && hasMA) {
        ImplicationGate cur = queue.front();
        queue.pop_front();
        cout << "cur: " << cur.ptr->name << " = " << cur.ptr->value  << endl;
        if ( cur.action == FORWARD ){
            cout << "FORWARD" << endl;
            for ( Gate* fanout : cur.ptr->fan_out ){
                if ( fanout->type == PO )
                    continue;
                // check conflict
                if ( fanout->value != -1 ){
                    if ( fanout->getInput(cur.ptr).ctrlVal == 1 ){
                        if ( !fanout->getInput(cur.ptr).inverter ){
                            if ( fanout->value == 1 && cur.ptr->value == 0 ){
                                hasMA = false;
                                break;
                            }
                        } else{
                            if ( fanout->value == 1 && cur.ptr->value == 1 ){
                                hasMA = false;
                                break;
                            }
                        }
                    }
                    else if ( fanout->getInput(cur.ptr).ctrlVal == 0 ){
                        if ( !fanout->getInput(cur.ptr).inverter ){
                            if ( fanout->value == 0 && cur.ptr->value == 1 ){
                                hasMA = false;
                                break;
                            }
                        } else{
                            if ( fanout->value == 0 && cur.ptr->value == 0 ){
                                hasMA = false;
                                break;
                            }
                        }
                    }
                }
                else{
                    cout << "\tcur->fanout: " << fanout->name << endl;
                    if ( fanout->getInput(cur.ptr).ctrlVal == 1 ){
                        cout << "\t\tctrl1" << endl;
                        if ( !fanout->getInput(cur.ptr).inverter ){
                            cout << "\t\t\tno inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\t\tcur->val = 0" << endl;;
                                fanout->value = 0;
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                        }
                        else{
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 1 ){
                                cout << "\t\t\t\tcur->val = 1" << endl;;
                                fanout->value = 0;
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                        }
                    }
                    else if ( fanout->getInput(cur.ptr).ctrlVal == 0 ){
                        cout << "\t\tctrl0" << endl;
                        if ( !fanout->getInput(cur.ptr).inverter ){
                            cout << "\t\t\tno inverter" << endl;
                            if ( cur.ptr->value == 1 ){
                                cout << "\t\t\t\tcur->val = 1" << endl;;
                                fanout->value = 1;
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                        }
                        else{
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\t\tcur->val = 0" << endl;;
                                fanout->value = 1;
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                        }
                    }
                }
            }
        }
        else if ( cur.action == BACKWARD ){
            cout << "BACKWARD" << endl;
            for ( auto& fanin : cur.ptr->fan_in ){
                //check conflict
                cout << "\tcur->fanin: " << fanin.ptr->name << endl;
                if ( fanin.ptr->value != -1 ){
                    if ( fanin.ctrlVal == 1 ){
                        if ( !fanin.inverter ){
                            if ( cur.ptr->value == 1 && fanin.ptr->value == 0){
                                hasMA = false;
                                break;
                            }
                        }else {
                            if ( cur.ptr->value == 1 && fanin.ptr->value == 1){
                                hasMA = false;
                                break;
                            }
                        }
                    }
                    else if ( fanin.ctrlVal == 0 ){
                        if ( !fanin.inverter ){
                            if ( cur.ptr->value == 0 && fanin.ptr->value == 1){
                                hasMA = false;
                                break;
                            }
                        }else {
                            if ( cur.ptr->value == 0 && fanin.ptr->value == 0){
                                hasMA = false;
                                break;
                            }
                        }
                    }
                }
                else{
                    if ( fanin.ctrlVal == 1 ){
                        cout << "\t\tctrl1" << endl;
                        if ( !fanin.inverter ){
                            cout << "\t\t\tno inverter" << endl;
                            if ( cur.ptr->value == 1 ){
                                cout << "\t\t\tcur->val = 1" << endl;
                                fanin.ptr->value = 1;
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                        }else {
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 1 ){
                                cout << "\t\t\tcur->val = 1" << endl;
                                fanin.ptr->value = 0;
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                        }
                    }
                    else if ( fanin.ctrlVal == 0 ){
                        cout << "\t\tctrl0" << endl;
                        if ( !fanin.inverter ){
                            cout << "\t\t\tno inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\tcur->val = 0" << endl;
                                fanin.ptr->value = 0;
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                        }else {
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\tcur->val = 0" << endl;
                                fanin.ptr->value = 1;
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                        }
                    }
                }
            }
        }
    }

    cout << target->name << ": " << target->value << " ";
    for (unsigned int i = modifyList.size()-1; i >= pos ; --i){
        cout << modifyList[i]->name << ": " << modifyList[i]->value << " ";
        MASet.insert(GateWithValue({modifyList.back(),modifyList.back()->value}));
        modifyList.back()->value = -1;
        modifyList.pop_back();
    }
    cout << endl;
    if ( !hasMA ) cout << "no MA." << endl;
    return MASet;
}

void ThresholdNetwork::evalMandatoryAssignments(){
    std::set<GateWithValue> MA0;
    std::set<GateWithValue> MA1;
    /* --------------------------collect target------------------------ */
    for (auto gate : gatePool) {
        if (!gate.second->sideInputs.empty())
            targetGateList.push_back(gate.second);
    }
    /* ------------iteratively eval mandotoryAssignments--------------- */
    cout << "evalMandatoryAssignments start." << endl;
    for (auto& t : gatePool) {
        Gate* target = t.second;
        modifyList.push_back(target);
        /* --------------------imply sideInput value------------------ */
        for ( Gate* sideInput : target->sideInputs )
            implySideInputVal( target, sideInput);
        /* ------------------------stuck at 0------------------------ */
        cout << "Start SA0" << endl;
        queue.clear();
        for ( Gate* modifiedGate : modifyList ){
            queue.push_back(ImplicationGate({modifiedGate,FORWARD}));
            queue.push_back(ImplicationGate({modifiedGate,BACKWARD}));
        }
        target->value = 1;  MA0 = iterativeImplication(target);
        /* ------------------------stuck at 1------------------------ */
        cout << "Start SA1" << endl;
        queue.clear();
        for ( Gate* modifiedGate : modifyList ){
            queue.push_back(ImplicationGate({modifiedGate,FORWARD}));
            queue.push_back(ImplicationGate({modifiedGate,BACKWARD}));
        }
        target->value = 0;  MA1 = iterativeImplication(target);
        std::set<GateWithValue> MA;
        // std::set_intersection(MA0.begin(), MA0.end(), MA1.begin(), MA1.end(),
        //     std::inserter(MA, MA.begin()));
        // cout << "MA Size: " << MA.size() << endl;

        cout << target->name << "\'s MASET_RES: " << endl;
        for ( auto it0 = MA0.begin() ; it0 != MA0.end(); ++it0 ){
            for ( auto it1 = MA1.begin(); it1 != MA1.end(); ++it1 ){
                if ( (*it0).ptr == (*it1).ptr ){
                    if ( (*it0).value != (*it1).value ){
                        MA.insert(GateWithValue({(*it1).ptr,-1}));
                    }
                }
            }
        }
        cout << endl;
        cout << "SIZE: " << MA.size() << endl;
        for ( auto& it : MA ){
            cout << it.ptr->name << " ";
        }
        cout << endl;
        cout << "********************************\n";
        for (const GateWithValue& ma : MA) {
            cout << ma.ptr->name << " ";
        }
        cout << endl;
        cout << "**************************************\n";
        for ( Gate* modifiedGate : modifyList )
            modifiedGate->value = -1;
        modifyList.clear();
    }
}

void ThresholdNetwork::implySideInputVal(Gate* target, Gate* sideInput ){
    for ( Gate* fanout : sideInput->fan_out ){
        if ( target->fanoutCone.find(fanout) != target->fanoutCone.end() ){
            if ( fanout->getInput(sideInput).ctrlVal != -1 ){
                if ( !fanout->getInput(sideInput).inverter ){
                    sideInput->value = fanout->getInput(sideInput).ctrlVal;
                }else {
                    sideInput->value = !fanout->getInput(sideInput).ctrlVal;
                }
                modifyList.push_back(sideInput);
            }
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

void ThresholdNetwork::_Debug_Check_The_Sum_Of_The_Number_Of_Side_Inputs(){
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
