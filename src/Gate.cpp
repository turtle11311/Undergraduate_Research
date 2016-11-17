#include "Gate.hpp"

Gate::Gate(const char* const name)
    : name(name)
{}

void Gate::addInput(Gate *input, int thresholdVal, bool phase)
{
    this->fan_in.push_back(ThresholdInput{input, thresholdVal, phase});
    input->fan_out.push_back(this);
}
