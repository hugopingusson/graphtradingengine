//
// Created by hugo on 08/04/25.
//

#include "HeartBeat.h"


HeartBeat::HeartBeat():SourceNode(),frequency() {}
HeartBeat::HeartBeat(const double &frequency):frequency(frequency) {
    this->name=fmt::format("HeartBeat(frequency={})", std::to_string(frequency));
}

double HeartBeat::get_frequency() const {
    return this->frequency;
}
