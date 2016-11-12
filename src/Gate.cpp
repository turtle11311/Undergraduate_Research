#include "Gate.hpp"

Gate::Gate(const char* const name)
    : name(name)
{}

void Gate::addInput(Gate *input, int thresholdVal)
{
    this->fan_in.push_back({input, thresholdVal, false});
    input->fan_out.push_back(this);
}
