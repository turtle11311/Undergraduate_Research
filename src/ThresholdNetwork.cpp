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
    /*-----------------------------------------------------------
    add inverter
    side input assign value
    each gate find MA
    side input, side input implied MA reuse
    -----------------------------------------------------------*/
    std::set<Gate*> MA0;
    std::set<Gate*> MA1;
    for (auto gate : gatePool) {
        if (!gate.second->sideInputs.empty())
            targetGateList.push_back(gate.second);
    }
    std::list<ImplacationGate> queue;
    cout << "evalMandatoryAssignments start." << endl;
    for (Gate* target : targetGateList) {
        cout << "Target: " << target->name << endl;
        bool hasMA = true;
        std::list<Gate*> modifyList;
        // Stuck at 0
        target->value = 1;
        ImplacationGate impGate;
        impGate.ptr = target;
        impGate.action = BACKWARD;
        queue.push_back(impGate);
        if ( queue.back().action == BACKWARD ) cout << "FUCK1" <<endl;
        impGate.action = FORWARD;
        queue.push_back(impGate);
        if ( queue.back().action == FORWARD ) cout << "FUCK2" <<endl;
        cout << "Stuck at 0 start." << endl;
        int counting = 0;
        while (!queue.empty() && hasMA) {
            cout << "count" << ++counting << endl;
            ImplacationGate nowGate = queue.front();
            queue.pop_front();
            cout << ( ( nowGate.action == FORWARD ) ? "FORWARD" : "BACKWARD" )<< endl;
            if ( nowGate.action == FORWARD ){
                cout << "FORWARD" << endl;
                for (Gate* fanout : nowGate.ptr->fan_out) {
                    if (fanout->value != -1) {
                        // fanout's value == 1
                        if (fanout->value == 1) {
                            // 1 ctrl input
                            if (fanout->getInput(nowGate.ptr).ctrlVal == 1) {
                                // has inverter
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                            else { // 0 ctrl input
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                        } else { // fanout's value == 0
                            // 1 ctrl input
                            if ( fanout->getInput(nowGate.ptr).ctrlVal == 1 ){
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                            else{ // 0 ctrl input
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                        }
                        queue.push_back(ImplacationGate({ fanout, BACKWARD }));
                        continue;
                    }
                    // Have three states: no ctrlVal, same ctrlVal and not same ctrlVal
                    if (fanout->getInput(nowGate.ptr).ctrlVal == -1) {
                        // DO NOTHING
                    } else {
                        int complement = !nowGate.ptr->value;
                        if ( fanout->getInput(nowGate.ptr).inverter ){
                            if (fanout->getInput(nowGate.ptr).ctrlVal == complement) {
                                cout << "Implied." << endl;
                                fanout->value = complement;
                                modifyList.push_back(fanout);
                                queue.push_back(ImplacationGate({fanout, FORWARD}));
                            } else{ // can't imply
                            /*Refresh fanout gate CEV */
                            //not sure
                            //fanout->thresholdVal -= fanout->getInput(nowGate.ptr).weight;
                            //fanout->getInput(nowGate.ptr).weight = 0;
                            //fanout->evalCriticalEffectVectors();
                            //fanout->checkContollingValueState(0);
                            //fanout->checkContollingValueState(1);
                            /*should do something,
                            put this fanout into queue, or put current gate into queue again?*/
                            }
                        } else {
                            if (fanout->getInput(nowGate.ptr).ctrlVal == nowGate.ptr->value) {
                                cout << "Implied." << endl;
                                fanout->value = nowGate.ptr->value;
                                modifyList.push_back(fanout);
                                queue.push_back(ImplacationGate({fanout, FORWARD}));
                            } else{ // can't imply
                                /*Refresh fanout gate CEV */
                                //not sure
                                //fanout->thresholdVal -= fanout->getInput(nowGate.ptr).weight;
                                //fanout->getInput(nowGate.ptr).weight = 0;
                                //fanout->evalCriticalEffectVectors();
                                //fanout->checkContollingValueState(0);
                                //fanout->checkContollingValueState(1);
                                /*should do something,
                                put this fanout into queue, or put current gate into queue again?*/
                            }
                        }
                    }
                }
            }
            else if ( nowGate.action == BACKWARD ){
                cout << "BACKWARD" << endl;
                // if nowGate have been Refresh the cevtable, should be put into queue again
                //bool doBackwardAgain = false;
                for (auto &fanin : nowGate.ptr->fan_in) {
                    if (nowGate.ptr->value == 0) {
                        // this fanin
                        if (fanin.ctrlVal != -1) {
                            // this fanin has value
                            if (fanin.ptr->value == -1) {
                                fanin.ptr->value = (!fanin.inverter)? 0 : 1;
                                modifyList.push_back(fanin.ptr);
                                queue.push_back(ImplacationGate({fanin.ptr, BACKWARD}));
                                queue.push_back(ImplacationGate({fanin.ptr, FORWARD}));
                            }
                            else {
                                if ( !fanin.inverter ){
                                    // check conflict
                                    if (fanin.ptr->value == 1) {
                                        hasMA = false;
                                        queue.clear();
                                        // add flag
                                    }
                                    else{ // Refresh cevtable
                                        // nowGate->thresholdVal -= fanin.weight;
                                        // fanin.weight = 0;
                                        // nowGate->evalCriticalEffectVectors();
                                        // nowGate->checkContollingValueState(0);
                                        // nowGate->checkContollingValueState(1);
                                        // doBackwardAgain = true;
                                    }
                                }
                                else {
                                    if (fanin.ptr->value == 0) {
                                        hasMA = false;
                                        queue.clear();
                                        // add flag
                                    }
                                    else{
                                        // nowGate->thresholdVal -= fanin.weight;
                                        // fanin.weight = 0;
                                        // nowGate->evalCriticalEffectVectors();
                                        // nowGate->checkContollingValueState(0);
                                        // nowGate->checkContollingValueState(1);
                                        // doBackwardAgain = true;
                                    }
                                }
                            }
                        }
                    }
                    //else if(nowGate.ptr->value == 1)
                    else {
                        // indirect imply
                    }
                }
                queue.push_back(ImplacationGate({nowGate.ptr,BACKWARD}));
            }
            else {
            }

            if (!queue.empty()) queue.pop_front();
        }
        MA0.insert(modifyList.begin(), modifyList.end());
        cout << "MA0--------------------------------------------------" << endl;
        for (Gate* gate : modifyList) {
            cout << gate->name << ": " << gate->value << " ";
            gate->value = -1;
        }
        cout << endl;
        modifyList.clear();

        hasMA = true;
        queue.clear();
        // Stuck at 1
        target->value = 0;
        queue.push_back(ImplacationGate({ target, FORWARD}));
        queue.push_back(ImplacationGate({ target, BACKWARD}));

        cout << "Stuck at 1 start." << endl;
        while (!queue.empty() && hasMA) {
            auto nowGate = queue.front();
            queue.pop_front();
            switch (nowGate.action) {
            case FORWARD:
                for (Gate* fanout : nowGate.ptr->fan_out) {
                    if (fanout->value != -1) {
                        // fanout's value == 1
                        if (fanout->value == 1) {
                            // 1 ctrl input
                            if (fanout->getInput(nowGate.ptr).ctrlVal == 1) {
                                // has inverter
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                            else { // 0 ctrl input
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 0){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                        } else { // fanout's value == 0
                            // 1 ctrl input
                            if ( fanout->getInput(nowGate.ptr).ctrlVal == 1 ){
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                            else{ // 0 ctrl input
                                if (fanout->getInput(nowGate.ptr).inverter){
                                    if ((!nowGate.ptr->value) == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                                else{
                                    if (nowGate.ptr->value == 1){
                                        hasMA = false;
                                        queue.clear();
                                        break;
                                    }
                                }
                            }
                        }
                        queue.push_back(ImplacationGate({ fanout, BACKWARD }));
                        continue;
                    }
                    // Have three states: no ctrlVal, same ctrlVal and not same ctrlVal
                    if (fanout->getInput(nowGate.ptr).ctrlVal == -1) {
                        // DO NOTHING
                    } else {
                        int complement = !nowGate.ptr->value;
                        if ( fanout->getInput(nowGate.ptr).inverter ){
                            if (fanout->getInput(nowGate.ptr).ctrlVal == complement) {
                                fanout->value = complement;
                                modifyList.push_back(fanout);
                                queue.push_back(ImplacationGate({fanout, FORWARD}));
                            } else{ // can't imply
                            /*Refresh fanout gate CEV */
                            //not sure
                            //fanout->thresholdVal -= fanout->getInput(nowGate.ptr).weight;
                            //fanout->getInput(nowGate.ptr).weight = 0;
                            //fanout->evalCriticalEffectVectors();
                            //fanout->checkContollingValueState(0);
                            //fanout->checkContollingValueState(1);
                            /*should do something,
                            put this fanout into queue, or put current gate into queue again?*/
                            }
                        } else {
                            if (fanout->getInput(nowGate.ptr).ctrlVal == nowGate.ptr->value) {
                                fanout->value = nowGate.ptr->value;
                                modifyList.push_back(fanout);
                                queue.push_back(ImplacationGate({fanout, FORWARD}));
                            } else{ // can't imply
                                /*Refresh fanout gate CEV */
                                //not sure
                                //fanout->thresholdVal -= fanout->getInput(nowGate.ptr).weight;
                                //fanout->getInput(nowGate.ptr).weight = 0;
                                //fanout->evalCriticalEffectVectors();
                                //fanout->checkContollingValueState(0);
                                //fanout->checkContollingValueState(1);
                                /*should do something,
                                put this fanout into queue, or put current gate into queue again?*/
                            }
                        }
                    }
                }
                break;
            case BACKWARD:
                // if nowGate have been Refresh the cevtable, should be put into queue again
                //bool doBackwardAgain = false;
                for (auto &fanin : nowGate.ptr->fan_in) {
                    if (nowGate.ptr->value == 0) {
                        // this fanin
                        if (fanin.ctrlVal != -1) {
                            // this fanin has value
                            if (fanin.ptr->value == -1) {
                                fanin.ptr->value = (!fanin.inverter)? 0 : 1;
                                modifyList.push_back(fanin.ptr);
                                queue.push_back(ImplacationGate({fanin.ptr, BACKWARD}));
                                queue.push_back(ImplacationGate({fanin.ptr, FORWARD}));
                            }
                            else {
                                if ( !fanin.inverter ){
                                    // check conflict
                                    if (fanin.ptr->value == 1) {
                                        hasMA = false;
                                        queue.clear();
                                        // add flag
                                    }
                                    else{ // Refresh cevtable
                                        // nowGate->thresholdVal -= fanin.weight;
                                        // fanin.weight = 0;
                                        // nowGate->evalCriticalEffectVectors();
                                        // nowGate->checkContollingValueState(0);
                                        // nowGate->checkContollingValueState(1);
                                        // doBackwardAgain = true;
                                    }
                                }
                                else {
                                    if (fanin.ptr->value == 0) {
                                        hasMA = false;
                                        queue.clear();
                                        // add flag
                                    }
                                    else{
                                        // nowGate->thresholdVal -= fanin.weight;
                                        // fanin.weight = 0;
                                        // nowGate->evalCriticalEffectVectors();
                                        // nowGate->checkContollingValueState(0);
                                        // nowGate->checkContollingValueState(1);
                                        // doBackwardAgain = true;
                                    }
                                }
                            }
                        }
                    }
                    //else if(nowGate.ptr->value == 1)
                    else {
                        // indirect imply
                    }
                }
                queue.push_back(ImplacationGate({nowGate.ptr,BACKWARD}));
                break;
            default:
                break;
            }
            if (!queue.empty()) queue.pop_front();
        }
        MA1.insert(modifyList.begin(), modifyList.end());
        cout << "MA1--------------------------------------------------" << endl;
        for (Gate* gate : modifyList) {
            cout << gate->name << ": " << gate->value << " ";
            gate->value = -1;
        }
        cout << endl;
        modifyList.clear();
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
