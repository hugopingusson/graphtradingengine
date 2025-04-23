//
// Created by hugo on 06/04/25.
//

#ifndef BACKTESTENGINE_H
#define BACKTESTENGINE_H

#include <regex>

#include "../Node/Base/Node.h"
#include "../Node/Base/MarketNode.h"
#include "../Streamer/Streamer.h"
#include "../Graph/Event.h"
#include "../Graph/Graph.h"


class BacktestEngine {
    public:
    BacktestEngine();
    BacktestEngine(Logger* logger,Graph* graph);

    Graph* get_graph();
    Logger* get_logger();
    BackTestStreamerContainer get_streamer_container();

    void initialize();
    void build_streamer_container();

    void run(const string& date);

    protected:
    Graph* graph;
    Logger* logger;
    BackTestStreamerContainer streamer_container;

};



#endif //BACKTESTENGINE_H
