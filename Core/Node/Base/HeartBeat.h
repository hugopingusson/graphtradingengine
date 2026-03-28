//
// Created by hugo on 08/04/25.
//

#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "Node.h"

// class HeartBeat : public SourceNode<HeartBeat> {
class HeartBeat: public Producer {
    public:
    HeartBeat();
    ~HeartBeat() override = default;
    HeartBeat(const double& frequency);

    double get_frequency() const;
    // void handle(HeartBeatEvent& heart_beat_event);
    void on_event(Event* event) override;

    protected:
    double frequency;


};



#endif //HEARTBEAT_H
