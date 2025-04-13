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

// void HeartBeat::handle(HeartBeatEvent& heart_beat_event) {
void HeartBeat::on_event(Event* event) {
    HeartBeatEvent* heart_beat_event = dynamic_cast<HeartBeatEvent*>(event);
    this->last_streamer_in_timestamp=heart_beat_event->get_last_streamer_in_timestamp();

}
