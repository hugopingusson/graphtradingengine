//
// Created by hugo on 08/04/25.
//

#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "../Base/Node.h"

class HeartBeat : public SourceNode {
    public:
    HeartBeat();
    ~HeartBeat() override = default;
    HeartBeat(const double& frequency);

    double get_frequency() const;
    void update_event(HeartBeatEvent* trade);
    void update() override;

    protected:
    double frequency;


};



#endif //HEARTBEAT_H
