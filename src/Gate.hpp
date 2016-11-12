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
#include <list>
#include <tuple>
#include <utility>
class Gate;
/*!
 * \typedef std::pair<Gate*, int> ThresholdInput
 * First is Gate's pointer, and second is its threshold value
 */
typedef std::tuple<Gate*, int, bool> ThresholdInput;

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
     * \brief Add input gate
     */
    void addInput(Gate* input, int thresholdVal);
    std::string name;                   /*!< Name of the gate */
    std::list<ThresholdInput> fan_in;   /*!< List of gate's inputs */
    std::list<Gate*> fan_out;           /*!< List of gate's outputs */

};
