/*!
 * @file
 *
 * @Author TurtleBee <turtle11311@gmail.com>
 * @section DESCRIPTION
 *
 * The class for threshold gate network.
 */
#pragma once
#include <string>
#include <list>
#include <utility>
class Gate;
/*!
 * \var \typedef std::pair<Gate*, int> ThresholdInput
 * First is Gate's pointer, and second is its threshold value
 */
typedef std::pair<Gate*, int> ThresholdInput;

/*!
 * \class Gate Gate.hpp
 * \brief The class for threshold gate network.
 */
class Gate {
public:
    /*!
     * \fn Gate(std::string name = "")
     * \brief Constructor of Gate
     * \param name for new gate
     */
    Gate(std::string name = "");
    std::string name;                   /*!< Name of the gate */
    std::list<ThresholdInput> fan_in;   /*!< List of gate's inputs */
    std::list<Gate*> fan_out;           /*!< List of gate's outputs */

};
