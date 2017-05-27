#include "ThresholdNetwork.hpp"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <functional>
#include <list>

using std::cout;
using std::endl;


ThresholdNetwork::ThresholdNetwork():indirectMode(false){
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
    for (auto &gate : gatePool){
        gate.second->fanoutCone.erase(gate.second);
        for ( auto& ti : gate.second->fan_in ){
            gate.second->constTotalWeight += ti.weight;
        }
        gate.second->totalWeight = gate.second->constTotalWeight;
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
    if ( indirectMode ) cout << "****************indirectMode******************" << endl;
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
                    std::vector<Gate*> implyGateList = fanout->backwardChecking();
                    for ( Gate* implyGate : implyGateList ){
                        implyGate->value = fanout->value == 1 ? 1 : 0;
                        queue.push_back(ImplicationGate({implyGate,FORWARD}));
                        queue.push_back(ImplicationGate({implyGate,BACKWARD}));
                        modifyList.push_back(implyGate);
                    }
                }
                else{
                    cout << "\tcur->fanout: " << fanout->name << endl;
                    if ( fanout->getInput(cur.ptr).ctrlVal == 1 ){
                        cout << "\t\tctrl1" << endl;
                        if ( !fanout->getInput(cur.ptr).inverter ){
                            cout << "\t\t\tno inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\t\tcur->val = 0" << endl;
                                fanout->value = 0;
                                cout << "\t\t\t\t\timply " << fanout->name << " = " << 0 << endl;
                                //fanout->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                            else{
                                fanout->totalWeight = fanout->constTotalWeight;
                                int directEvalResTemp = fanout->directEvalRes();
                                if ( directEvalResTemp == -1)
                                    cout << "\t\t\tthis gate's value can't determined.\n";
                                else {
                                    cout << "\t\t\tthis fanout's all fanin has been determined.\n";
                                    fanout->value = directEvalResTemp ? 1 : 0;
                                    cout << "\t\t\t\t\timply " << fanout->name << " = " << fanout->value << endl;
                                    queue.push_back(ImplicationGate({fanout,FORWARD}));
                                    queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                        }
                        else{
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 1 ){
                                cout << "\t\t\t\tcur->val = 1" << endl;
                                fanout->value = 0;
                                cout << "\t\t\t\t\timply " << fanout->name << " = " << 0 << endl;
                                //fanout->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                            else{
                                fanout->totalWeight = fanout->constTotalWeight;
                                int directEvalResTemp = fanout->directEvalRes();
                                if ( directEvalResTemp == -1)
                                    cout << "\t\t\tthis gate's value can't determined.\n";
                                else {
                                    cout << "\t\t\tthis fanout's all fanin has been determined.\n";
                                    fanout->value = directEvalResTemp ? 1 : 0;
                                    cout << "\t\t\t\t\timply " << fanout->name << " = " << fanout->value << endl;
                                    queue.push_back(ImplicationGate({fanout,FORWARD}));
                                    queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                        }
                    }
                    else if ( fanout->getInput(cur.ptr).ctrlVal == 0 ){
                        cout << "\t\tctrl0" << endl;
                        if ( !fanout->getInput(cur.ptr).inverter ){
                            cout << "\t\t\tno inverter" << endl;
                            if ( cur.ptr->value == 1 ){
                                cout << "\t\t\t\tcur->val = 1" << endl;
                                fanout->value = 1;
                                cout << "\t\t\t\t\timply " << fanout->name << " = " << 1 << endl;
                                //fanout->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                            else{
                                fanout->totalWeight = fanout->constTotalWeight;
                                int directEvalResTemp = fanout->directEvalRes();
                                if ( directEvalResTemp == -1)
                                    cout << "\t\t\tthis gate's value can't determined.\n";
                                else {
                                    cout << "\t\t\tthis fanout's all fanin has been determined.\n";
                                    fanout->value = directEvalResTemp ? 1 : 0;
                                    cout << "\t\t\t\t\timply " << fanout->name << " = " << fanout->value << endl;
                                    queue.push_back(ImplicationGate({fanout,FORWARD}));
                                    queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
                            }
                        }
                        else{
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\t\tcur->val = 0" << endl;
                                fanout->value = 1;
                                cout << "\t\t\t\t\timply " << fanout->name << " = " << 1 << endl;
                                //fanout->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanout,FORWARD}));
                                queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                modifyList.push_back(fanout);
                            }
                            else{
                                fanout->totalWeight = fanout->constTotalWeight;
                                int directEvalResTemp = fanout->directEvalRes();
                                if ( directEvalResTemp == -1)
                                    cout << "\t\t\tthis gate's value can't determined.\n";
                                else {
                                    cout << "\t\t\tthis fanout's all fanin has been determined.\n";
                                    fanout->value = directEvalResTemp ? 1 : 0;
                                    cout << "\t\t\t\t\timply " << fanout->name << " = " << fanout->value << endl;
                                    queue.push_back(ImplicationGate({fanout,FORWARD}));
                                    queue.push_back(ImplicationGate({fanout,BACKWARD}));
                                    modifyList.push_back(fanout);
                                }
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
                                cout << "\t\t\t\t\timply " << fanin.ptr->name << " = " << 1 << endl;
                                //fanin.ptr->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                            // else if ( cur.ptr->evalIndirectImnplicationList(1)-1 < Gate::indirectLevelConstraint ){
                            //     cout << "WTF" << endl;
                            //     indirectMode = true;
                            //     indirectList.push_back(cur.ptr);
                            // }
                        }else {
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 1 ){
                                cout << "\t\t\tcur->val = 1" << endl;
                                fanin.ptr->value = 0;
                                cout << "\t\t\t\t\timply " << fanin.ptr->name << " = " << 0 << endl;
                                //fanin.ptr->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                            // else if ( cur.ptr->evalIndirectImnplicationList(1)-1 < Gate::indirectLevelConstraint ){
                            //     cout << "WTF" << endl;
                            //     indirectMode = true;
                            //     indirectList.push_back(cur.ptr);
                            // }
                        }
                    }
                    else if ( fanin.ctrlVal == 0 ){
                        cout << "\t\tctrl0" << endl;
                        if ( !fanin.inverter ){
                            cout << "\t\t\tno inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\tcur->val = 0" << endl;
                                fanin.ptr->value = 0;
                                cout << "\t\t\t\t\timply " << fanin.ptr->name << " = " << 0 << endl;
                                //fanin.ptr->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                            // else if ( cur.ptr->evalIndirectImnplicationList(0)-1 < Gate::indirectLevelConstraint ){
                            //     indirectMode = true;
                            //     indirectList.push_back(cur.ptr);
                            // }
                        }else {
                            cout << "\t\t\thas inverter" << endl;
                            if ( cur.ptr->value == 0 ){
                                cout << "\t\t\tcur->val = 0" << endl;
                                fanin.ptr->value = 1;
                                cout << "\t\t\t\t\timply " << fanin.ptr->name << " = " << 1 << endl;
                                //fanin.ptr->refreshDeterminedFaninCount();
                                queue.push_back(ImplicationGate({fanin.ptr,FORWARD}));
                                queue.push_back(ImplicationGate({fanin.ptr,BACKWARD}));
                                modifyList.push_back(fanin.ptr);
                            }
                            // else if ( cur.ptr->evalIndirectImnplicationList(0)-1 < Gate::indirectLevelConstraint ){
                            //     indirectMode = true;
                            //     indirectList.push_back(cur.ptr);
                            // }
                        }
                    }
                }
            }
        }
    }
    if ( !indirectMode )
        reinitializeModifiyList(pos, MASet,target);
    if ( !hasMA ) cout << "no MA." << endl;
    return MASet;
}

void ThresholdNetwork::reinitializeModifiyList( int pos, std::set<GateWithValue>& MASet, Gate* target ){
    //cout << target->name << ": " << target->value << " ";
    for (unsigned int i = 0; i < modifyList.size() ; ++i)
        MASet.insert(GateWithValue({modifyList[i],modifyList[i]->value}));
    for (unsigned int i = modifyList.size()-1; i >= pos ; --i){
    //    cout << modifyList[i]->name << ": " << modifyList[i]->value << " ";
        modifyList.back()->value = -1;
        modifyList.pop_back();
    }
}

void ThresholdNetwork::_Debug_Mandatory_Assignments( std::set<GateWithValue>& MA){
    for ( GateWithValue gwv : MA ){
        cout << gwv.ptr->name << ": " << gwv.value << ", ";
    }
    cout << endl;
}

void ThresholdNetwork::evalMandatoryAssignments(){
    cout << "evalMandatoryAssignments start." << endl;
    std::set<GateWithValue> MA0;
    std::set<GateWithValue> MA1;
    /* --------------------------collect target------------------------ */
    for (auto gate : gatePool) {
        if (!gate.second->sideInputs.empty())
            targetGateList.push_back(gate.second);
    }
    /* ------------iteratively eval mandotoryAssignments--------------- */
    for (auto& t : gatePool) {
        Gate* target = t.second;
        modifyList.push_back(target);
        /* --------------------imply sideInput value------------------ */
        for ( Gate* sideInput : target->sideInputs )
            implySideInputVal( target, sideInput);
        /* ------------------------stuck at 0------------------------ */
        cout << "Start SA0" << endl;
        cout << "cur modifyList size: " << modifyList.size() <<endl;
        target->value = 1;
        //target->refreshDeterminedFaninCount(true);
        queue.clear();
        cout << "Target " << target->name << "\'s initialMA: ";
        for ( Gate* modifiedGate : modifyList ){
            cout << modifiedGate->name << ": " << modifiedGate->value << ", ";
            queue.push_back(ImplicationGate({modifiedGate,FORWARD}));
            queue.push_back(ImplicationGate({modifiedGate,BACKWARD}));
        }
        cout << endl;
        int initialListSize = modifyList.size();
        MA0 = iterativeImplication(target);
        cout << target->name << "\'s MA0-> ";
        _Debug_Mandatory_Assignments(MA0);
        // intersectionOfIndirectTarget(target, MA0);
        // reinitializeModifiyList(initialListSize,MA0,target);
        // if ( indirectMode ) indirectMode = false;
        /* ------------------------stuck at 1------------------------ */
        cout << "Start SA1" << endl;
        cout << "cur modifyList size: " << modifyList.size() <<endl;
        target->value = 0;
        queue.clear();
        for ( Gate* modifiedGate : modifyList ){
            queue.push_back(ImplicationGate({modifiedGate,FORWARD}));
            queue.push_back(ImplicationGate({modifiedGate,BACKWARD}));
        }
        MA1 = iterativeImplication(target);
        cout << target->name << "\'s MA1-> ";
        _Debug_Mandatory_Assignments(MA1);
        // intersectionOfIndirectTarget(target, MA1);
        // reinitializeModifiyList(initialListSize,MA1,target);
        // if ( indirectMode ) indirectMode = false;

        std::set<Gate*> MA;
        cout << target->name << "\'s MASET_RES: " << endl;
        for ( auto it0 = MA0.begin() ; it0 != MA0.end(); ++it0 ){
            for ( auto it1 = MA1.begin(); it1 != MA1.end(); ++it1 ){
                if ( (*it0).ptr == (*it1).ptr ){
                    if ( (*it0).value != (*it1).value ){
                        MA.insert((*it1).ptr);
                    }
                }
            }
        }
        MA.erase(target);
        cout << endl;
        cout << "SIZE: " << MA.size() << endl;
        for (Gate* ma : MA) {
            cout << ma->name << " ";
        }
        cout << endl;
        cout << "**************************************\n";
        for ( Gate* modifiedGate : modifyList )
            modifiedGate->value = -1;
        modifyList.clear();
    }
}

void ThresholdNetwork::intersectionOfIndirectTarget( Gate* target , std::set<GateWithValue>& MA){
    if ( indirectMode ){
        for ( Gate* indirectImplicationTarget : indirectList ){
            if ( indirectImplicationTarget->value == 1 ){
                std::vector<std::set<GateWithValue>> indirectTable;
                for ( int i = 0 ; i < indirectImplicationTarget->onsetStage ; ++i ){
                    indirectImplicationTarget->fan_in[i].ptr->value = 1;
                    //indirectImplicationTarget->fan_in[i].ptr->refreshDeterminedFaninCount(true);
                    int modifyListSize = modifyList.size();
                    queue.clear();
                    queue.push_back(ImplicationGate({indirectImplicationTarget->fan_in[i].ptr,FORWARD}));
                    queue.push_back(ImplicationGate({indirectImplicationTarget->fan_in[i].ptr,BACKWARD}));
                    std::set<GateWithValue> tempSet = iterativeImplication(target);
                    if (!tempSet.size())
                        continue;

                    std::set<GateWithValue> modifyListJr;
                    for ( int i = modifyListSize ; i < modifyList.size(); ++i )
                    modifyListJr.insert(GateWithValue({modifyList[i],modifyList[i]->value}));
                    indirectTable.push_back(modifyListJr);
                    indirectImplicationTarget->fan_in[i].ptr->value = -1;
                    //indirectImplicationTarget->fan_in[i].ptr->refreshDeterminedFaninCount(false);
                    for ( int i = modifyList.size()-1 ; i >= modifyListSize ; --i ){
                        modifyList[i]->value = -1;
                        modifyList.pop_back();
                    }
                }
                if ( !indirectTable.size() ) continue;
                for ( auto& indirectGateValue : indirectTable[0] ){
                    bool haveSameOne = true;
                    for ( int j = 1 ; j < indirectTable.size() ; ++j){
                        if ( indirectTable[j].find( indirectGateValue ) != indirectTable[j].end() ){
                            haveSameOne = false;
                            break;
                        }
                    }
                    if ( haveSameOne )
                        MA.insert(indirectGateValue);
                }
            }

            if ( indirectImplicationTarget->value == 0 ){
                std::vector<std::set<GateWithValue>> indirectTable;
                for ( int i = 0 ; i < indirectImplicationTarget->offsetStage ; ++i ){
                    indirectImplicationTarget->fan_in[i].ptr->value = 0;
                    int modifyListSize = modifyList.size();
                    queue.clear();
                    queue.push_back(ImplicationGate({indirectImplicationTarget->fan_in[i].ptr,FORWARD}));
                    queue.push_back(ImplicationGate({indirectImplicationTarget->fan_in[i].ptr,BACKWARD}));
                    std::set<GateWithValue> tempSet = iterativeImplication(target);
                    if (!tempSet.size())
                        continue;
                    std::set<GateWithValue> modifyListJr;
                    for ( int i = modifyListSize ; i < modifyList.size(); ++i )
                    modifyListJr.insert(GateWithValue({modifyList[i],modifyList[i]->value}));
                    indirectTable.push_back(modifyListJr);
                    indirectImplicationTarget->fan_in[i].ptr->value = -1;
                    for ( int i = modifyList.size()-1 ; i >= modifyListSize ; --i ){
                        modifyList[i]->value = -1;
                        modifyList.pop_back();
                    }
                }
                if ( !indirectTable.size() ) continue;
                for ( auto& indirectGateValue : indirectTable[0] ){
                    bool haveSameOne = true;
                    for ( int j = 1 ; j < indirectTable.size() ; ++j){
                        if ( indirectTable[j].find( indirectGateValue ) != indirectTable[j].end() ){
                            haveSameOne = false;
                            break;
                        }
                    }
                    if ( haveSameOne )
                        MA.insert(indirectGateValue);
                }
            }
        }
    }
    cout << "indirectMA: " << MA.size() << endl;
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
                sideInput->refreshDeterminedFaninCount(true);
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
        gate.second->determinedFaninCount = gate.second->fan_in.size();
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
