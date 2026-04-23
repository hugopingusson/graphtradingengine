//
// Created by hugo on 03/04/26.
//

#ifndef LIVEENGINE_H
#define LIVEENGINE_H

#include <atomic>
#include <map>
#include <memory>

#include "../Graph/Graph.h"
#include "../Streamer/LiveStreamer.h"

class LiveEngine {
public:
    LiveEngine();
    LiveEngine(Logger* logger, Graph* graph);
    ~LiveEngine();

    Graph* get_graph() const;
    Logger* get_logger() const;

    void add_streamer(std::unique_ptr<LiveStreamer> streamer);
    const std::map<size_t, std::unique_ptr<LiveStreamer>>& get_streamers() const;
    LiveStreamer* get_streamer(const size_t& streamer_id) const;

    void initialize();
    void build_streamer_container();
    void run();

    void start();
    void stop();
    void join();
    bool is_running() const;

private:
    void register_source(const Producer* source_node);
    void register_market_source(const Market* market);

    void run_consumer_loop();
    void process_event(LiveStreamer::EventPtr event);

    Logger* logger;
    Graph* graph;
    size_t max_streamer_id;
    std::map<size_t, std::unique_ptr<LiveStreamer>> streamers;
    std::atomic<bool> running;
};

#endif //LIVEENGINE_H
