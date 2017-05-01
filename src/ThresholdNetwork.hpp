/*!
 * @file
 *
 * @Author TurtleBee <turtle11311@gmail.com>
 * @Author Yaimaoz <ny8859884@gmail.com>
 * @section DESCRIPTION
 *
 * The class for threshold gate network
 */
#pragma once
#include "Gate.hpp"
#include <map>
#include <list>
/*!
 * \typedef std::map<std::string, Gate*> GateDict
 * A storage support quick query by the name
 */
typedef std::map<std::string, Gate*> GateDict;

enum ImplicationType { FORWARD, BACKWARD };

struct ImplicationGate {
    Gate *ptr;
    ImplicationType action;
};

struct GateWithValue {
    Gate* ptr;
    int value;
    bool operator<(const GateWithValue& rhs) const {
        if (this->ptr->name < rhs.ptr->name) {
            return true;
        } else if (this->ptr->name >= rhs.ptr->name) {
            return false;
        }
    }
};

class ThresholdNetworkDebugger;
/*!
 * \class ThresholdNetwork ThresholdNetwork.hpp
 * \brief The class for threshold gate network
 */
class ThresholdNetwork {
    friend class ThresholdNetworkDebugger;
private:
    GateDict gatePool;                      /*!< Dictonary for gate */
    Gate start;                             /*!< pseudo gate for all pi's fanin*/
    Gate end;                               /*!< pseudo gate for all po's fanout*/
    std::vector<Gate*> targetGateList;
    std::list<ImplicationGate> queue;
    std::vector<Gate*> modifyList;
    bool indirectMode;
    std::list<Gate*> indirectList;
public:
    /*!
     * \fn ThresholdNetwork()
     * \brief Constructor
     */
    ThresholdNetwork();
    /*!
     * \fn Gate* accessGateByName(const char* const name)
     * \param name name for new gate
     * \brief accessGateByName
     * If the gate names name is exist, then return the pointer to it
     * or it don't exist, then create this gate and return the pointer to it
     */
    Gate* accessGateByName(const char* const name);
    /*!
     * \fn gateClassify()
     * \brief classify all the gate
     */
    void gateClassify();
    /*!
     * \fn foreachGateAttr()
     * \brief generate essentail attribute for logic sythesis
     * evaluate dominators, CEVs, FanoutCone and side inputs for each gates in this network
     */
    void foreachGateAttr();

    void evalMandatoryAssignments();

    void implySideInputVal(Gate*,Gate*);

    std::set<GateWithValue> iterativeImplication( Gate*);
    void intersectionOfIndirectTarget(Gate*, std::set<GateWithValue>&);
    void reinitializeModifiyList( int, std::set<GateWithValue>& , Gate*);
    void _Debug_Wiring();
    void _Debug_Onset_Critical_Effect_Vector();
    void _Debug_Controlling_Value();
    void _Debug_Check_The_Sum_Of_The_Number_Of_Side_Inputs();
    /*!
     * \fn ~ThresholdNetwork()
     * \brief Destructor of this class
     * Release all memory to system
     */
    virtual ~ThresholdNetwork();
};
