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


void ThresholdNetwork::evalMandatoryAssignments(){
    std::set<Gate*> MA0;
    std::set<Gate*> MA1;
    /* --------------------------collect target------------------------ */
    for (auto gate : gatePool) {
        if (!gate.second->sideInputs.empty())
            targetGateList.push_back(gate.second);
    }
    /* ------------iteratively eval mandotoryAssignments--------------- */
    std::list<ImplacationGate> queue;
    cout << "evalMandatoryAssignments start." << endl;
    for (Gate* target : targetGateList) {
        cout << "Target: " << target->name << " | start."<< endl;
        cout << "Side number： " << target->sideInputs.size() << endl;
        bool hasMA = true;
        /* --------------------imply sideInput value------------------ */
        for ( Gate* sideInput : target->sideInputs )
            implySideInputVal( target, sideInput );
        /* ------------------------stuck at 0------------------------ */
        target->value = 1;
        modifyList.push_back(target);
        queue.push_back(ImplacationGate({ target, FORWARD}));
        queue.push_back(ImplacationGate({ target, BACKWARD}));
        cout << "Stuck at 0 start." << endl;
        while (!queue.empty() && hasMA) {
            ImplacationGate cur = queue.front();
            queue.pop_front();
            cout << "cur: " << cur.ptr->name << " = " << cur.ptr->value  << endl;
            if ( cur.action == FORWARD ){
                cout << "FORWARD" << endl;
                for ( Gate* fanout : cur.ptr->fan_out ){
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
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                            else{
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 1 ){
                                    cout << "\t\t\t\tcur->val = 1" << endl;;
                                    fanout->value = 0;
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
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
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                            else{
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 0 ){
                                    cout << "\t\t\t\tcur->val = 0" << endl;;
                                    fanout->value = 1;
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                        }
                        else if ( fanout->getInput(cur.ptr).ctrlVal == 2 ){
                            if ( !fanout->getInput(cur.ptr).inverter ){
                                fanout->value = cur.ptr->value;
                                queue.push_back(ImplacationGate({fanout,FORWARD}));
                                queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                            else{
                                fanout->value = !cur.ptr->value;
                                queue.push_back(ImplacationGate({fanout,FORWARD}));
                                queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
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
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                    modifyList.push_back(fanin.ptr);
                                }
                            }else {
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 1 ){
                                    cout << "\t\t\tcur->val = 1" << endl;
                                    fanin.ptr->value = 0;
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
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
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                    modifyList.push_back(fanin.ptr);
                                }
                            }else {
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 0 ){
                                    cout << "\t\t\tcur->val = 0" << endl;
                                    fanin.ptr->value = 1;
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                    modifyList.push_back(fanin.ptr);
                                }
                            }
                        }
                        else if ( fanin.ctrlVal == 2 ){
                            if ( !fanin.inverter ){
                                fanin.ptr->value = cur.ptr->value;
                                queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                            else{
                                fanin.ptr->value = !cur.ptr->value;
                                queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                        }
                    }
                }
            }
        }

        /* ------------------------reset modified gate------------------------ */
        MA0.insert(modifyList.begin(), modifyList.end());
        for (Gate* gate : modifyList) {
            cout << gate->name << ": " << gate->value << " ";
            gate->value = -1;
        }
        cout << endl;
        if ( !hasMA ) cout << "noMA0" << endl;
        modifyList.clear();

        hasMA = true;
        queue.clear();
        /* ------------------------stuck at 1------------------------ */
        target->value = 0;
        modifyList.push_back(target);
        queue.push_back(ImplacationGate({ target, FORWARD}));
        queue.push_back(ImplacationGate({ target, BACKWARD}));
        cout << "Stuck at 1 start." << endl;

        while (!queue.empty() && hasMA) {
            ImplacationGate cur = queue.front();
            queue.pop_front();
            cout << "cur: " << cur.ptr->name << " = " << cur.ptr->value  << endl;
            if ( cur.action == FORWARD ){
                cout << "FORWARD" << endl;
                for ( Gate* fanout : cur.ptr->fan_out ){
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
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                            else{
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 1 ){
                                    cout << "\t\t\t\tcur->val = 1" << endl;;
                                    fanout->value = 0;
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
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
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                            else{
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 0 ){
                                    cout << "\t\t\t\tcur->val = 0" << endl;;
                                    fanout->value = 1;
                                    queue.push_back(ImplacationGate({fanout,FORWARD}));
                                    queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                        }
                        else if ( fanout->getInput(cur.ptr).ctrlVal == 2 ){
                            if ( !fanout->getInput(cur.ptr).inverter ){
                                fanout->value = cur.ptr->value;
                                queue.push_back(ImplacationGate({fanout,FORWARD}));
                                queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                            else{
                                fanout->value = !cur.ptr->value;
                                queue.push_back(ImplacationGate({fanout,FORWARD}));
                                queue.push_back(ImplacationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
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
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                    modifyList.push_back(fanin.ptr);
                                }
                            }else {
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 1 ){
                                    cout << "\t\t\tcur->val = 1" << endl;
                                    fanin.ptr->value = 0;
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
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
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                    modifyList.push_back(fanin.ptr);
                                }
                            }else {
                                cout << "\t\t\thas inverter" << endl;
                                if ( cur.ptr->value == 0 ){
                                    cout << "\t\t\tcur->val = 0" << endl;
                                    fanin.ptr->value = 1;
                                    queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                    queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                    modifyList.push_back(fanin.ptr);
                                }
                            }
                        }
                        else if ( fanin.ctrlVal == 2 ){
                            if ( !fanin.inverter ){
                                fanin.ptr->value = cur.ptr->value;
                                queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                            else{
                                fanin.ptr->value = !cur.ptr->value;
                                queue.push_back(ImplacationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplacationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                        }
                    }
                }
            }
        }

        /* ------------------------reset modified gate------------------------ */
        MA1.insert(modifyList.begin(), modifyList.end());
        for (Gate* gate : modifyList) {
            cout << gate->name << ": " << gate->value << " ";
            gate->value = -1;
        }
        cout << endl;
        modifyList.clear();

        if ( !hasMA ) cout << "noMA1" << endl;
        sideInputModifyList.clear();

        cout << "Target: " << target->name << " | end."<< endl << endl;
    }
}

void ThresholdNetwork::implySideInputVal(Gate* target, Gate* sideInput ){

    //cout << "Side Input: " << sideInput->name << endl;
    for ( Gate* fanout : sideInput->fan_out ){
        // if sideInput's fanout in target's fanoutCone
        if ( target->fanoutCone.find(fanout) != target->fanoutCone.end() ){
            //cout << "Dominator: " << fanout->name << endl;
            if ( fanout->getInput(sideInput).ctrlVal == 1 || fanout->getInput(sideInput).ctrlVal == 0  ){
                if ( !fanout->getInput(sideInput).inverter )
                    sideInput->value = fanout->getInput(sideInput).ctrlVal;
                else
                    sideInput->value = !fanout->getInput(sideInput).ctrlVal;
            }
            sideInputModifyList.push_back(sideInput);
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
