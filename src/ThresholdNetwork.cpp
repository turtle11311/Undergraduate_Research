#include "ThresholdNetwork.hpp"
#include <iostream>

Gate* ThresholdNetwork::accessGateByName(const char *const name)
{
    GateDict::iterator gateIt = gatePool.find(name);
    Gate* nowGate = (gateIt != gatePool.end()) ? gateIt->second : new Gate(name);
    gatePool[name] = nowGate;
    return nowGate;
}

void ThresholdNetwork::_Debug_Wiring()
{
    for (auto& gate : gatePool) {
        if (!gate.second->fan_in.size())
            continue;
        std::cout << "Name: " << gate.first << "-> " << gate.second->thresholdVal << std::endl;
        std::cout << "FANIN: " << std::endl;
        for (auto& fanin : gate.second->fan_in) {
            std::cout << std::get<0>(fanin)->name << ": "
                      << std::get<1>(fanin) << std::endl;
        }
        std::cout << "FANOUT: " << std::endl;
        for (auto& fanout : gate.second->fan_out) {
            std::cout << fanout->name << ", ";
        }
        std::cout << std::endl << std::endl;
    }
}

ThresholdNetwork::~ThresholdNetwork()
{
    for (auto& gate : gatePool) {
        delete gate.second;
    }
}
