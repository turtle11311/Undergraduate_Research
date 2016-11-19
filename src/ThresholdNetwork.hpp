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
/*!
 * \typedef std::map<std::string, Gate*> GateDict
 * A storage support quick query by the name
 */
typedef std::map<std::string, Gate*> GateDict;

/*!
 * \class ThresholdNetwork ThresholdNetwork.hpp
 * \brief The class for threshold gate network
 */
class ThresholdNetwork {
private:
    GateDict gatePool;                      /*!< Dictonary for gate */
public:
    /*!
     * \fn Gate* accessGateByName(const char* const name)
     * \param name name for new gate
     * \brief accessGateByName
     * If the gate names name is exist, then return the pointer to it
     * or it don't exist, then create this gate and return the pointer to it
     */
    Gate* accessGateByName(const char* const name);
    /*!
     * \fn findCEVs(void)
     * \brief find all the onset/offset critical effect vectors
     */
    void findCEVs(void);
    void _Debug_Wiring();
    void _Debug_Onset_Critical_Effect_Vector();
    /*!
     * \fn ~ThresholdNetwork()
     * \brief Destructor of this class
     * Release all memory to system
     */
    virtual ~ThresholdNetwork();
};
