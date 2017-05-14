/*!
* @file
*
* @Author TurtleBee <turtle11311@gmail.com>
* @Author Yaimaoz <ny8859884@gmail.com>
* @section DESCRIPTION
*
* The class for threshold gate
*/
#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <set>
class Gate;

/*!
* \enum gate type
* First(0) Constant mean the gate has no fanin & fanout
* Second(1) Pi mean the gate has no fanin
* Third(2) Po mean the gate has no fanout
* Forth(3) Internal mean the gate has both fanin & fanout
*/
enum GateType { Constant, PI, PO, Internal };
/*!
* \typedef std::vector<std::vector<int>> PatternTable
* Storage of onset/offset critical effect vectors
*/
typedef std::vector<std::vector<int>> PatternTable;
/*!
* \struct ThresholdInput Gate.hpp
* \breif Threshold gate input
*/
struct ThresholdInput {
    Gate* ptr;          /*!< Pointer of input gate */
    int weight;         /*!< Weight of input */
    bool inverter;      /*!< This input has inverter */
    int ctrlVal;        /*!< Controlling value of input */
};
/*!
* \struct GateAttr Gate.hpp
* \brief The class for threshold gate in parsing
*/
struct GateAttr{
    char* name;
    int thresholdVal;
};
/*!
* \class Gate Gate.hpp
* \brief The class for threshold gate
*/
class Gate {
public:
    /*!
    * \fn Gate(const char* const name = "")
    * \param name name for new gate
    * \brief Constructor of Gate
    */
    Gate(const char* const name = "");
    /*!
    * \fn void addInput(Gate* input, int thresholdVal)
    * \param input pointer to input gate
    * \param thresholdVal threshold value for this input
    * \param phase this input have inverter?
    * \brief Add input gate
    */
    void addInput(Gate* input, int thresholdVal, bool phase);
    /*!
    * \fn void onsetCriticalEffectVector(std::vector<int> curPattern, int pos, int uncheckedSum)
    * \param current fan_in pattern
    * \param the check bit
    * \param current checked bits' weight sum
    * \param the unchecked bits' weight sum
    * \brief a recursive function to find all the "Onset Critical Effect Vector"
    */
    void onsetCriticalEffectVector(std::vector<int> curPattern, unsigned int pos, int curWeightSum, int uncheckedSum);
    /*!
     * \fn void offsetCriticalEffectVector(std::vector<int> curPattern, int pos, int uncheckedSum)
     * \param current fan_in pattern
     * \param the check bit
     * \param current checked bits' weight sum
     * \param the unchecked bits' weight sum
     * \brief a recursive function to find all the "Offset Critical Effect Vector"
     */
    void offsetCriticalEffectVector(std::vector<int> curPattern, unsigned int pos, int curWeightSum, int uncheckedSum);
    /*!
     * \fn evalDominators()
     * \brief evaluate dominators of this gate
     */
    std::set<Gate*>* evalDominators();
    /*!
     * \fn std::set<Gate*>* evalFanoutCone()
     * \brief evaluate fanoutCone of this gate
     */
    std::set<Gate*>* evalFanoutCone();
    /*!
     * \fn void evalCriticalEffectVectors()
     * \brief evaluate critical effect vectors of this gate
     */
    void evalCriticalEffectVectors();
    /*!
     * \fn void evalSideInput()
     * \brief evaluate side inputs of this gate
     */
    void evalSideInput();
    /*!
     * \fn void checkContollingValueState()
     * \brief check the 0-cev & 1-cev to get the controlling value state
     */
    void checkContollingValueState();
    void _Debug_Side_Inputs();
    void _Debug_Fanout_Cone();
    int evalIndirectImnplicationList( int mode );

    const ThresholdInput& getInput(const Gate* target);
    void _Debug_Gate_Information();
    int sideInputControllingValCount;       /*!< sum of number of sideInput's controlling Val*/
    int thresholdVal;                       /*!< Threshold value of this gate */
    GateType type;                          /*!< Type of the gate*/
    std::string name;                       /*!< Name of the gate */
    std::vector<ThresholdInput> fan_in;     /*!< List of gate's inputs */
    std::vector<Gate*> fan_out;             /*!< List of gate's outputs */
    PatternTable onsetTable;                /*!< Vector of vector of all the onset vectors*/
    PatternTable offsetTable;               /*!< Vector of vector of all the offset vectors*/
    std::set<Gate*> dominators;             /*!< A set of this gate's dominators */
    std::set<Gate*> fanoutCone;             /*!< A set of this gate's fanoutCone */
    std::set<Gate*> sideInputs;             /*!< A set of this gate's sideinputs */
    std::set<Gate*> mandotoryAssignments;   /*!< A set of this gate's mandotoryAssignments */
    short value;                            /*!< gate value */
    int onsetStage, offsetStage;
    const static int indirectLevelConstraint = 3;
};
