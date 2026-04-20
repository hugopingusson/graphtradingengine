//
// Created by hugo on 03/04/26.
//

#include "LiveEngine.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <limits>
#include <queue>
#include <string>
#include <stdexcept>
#include <vector>

#include <fmt/format.h>

#include "../../Helper/SaphirManager.h"
#include "../Streamer/BitmexLiveOrderBookStreamer.h"
#include "../Streamer/BinanceLiveOrderBookStreamer.h"
#include "../Streamer/DeribitLiveOrderBookStreamer.h"
#include "../Streamer/OkxLiveOrderBookStreamer.h"

LiveEngine::LiveEngine()
    : logger(nullptr),
      graph(nullptr),
      max_streamer_id(0),
      streamers(),
      running(false) {}

LiveEngine::LiveEngine(Logger* logger, Graph* graph)
    : logger(logger),
      graph(graph),
      max_streamer_id(0),
      streamers(),
      running(false) {}

LiveEngine::~LiveEngine() {
    this->stop();
    this->join();
}

Graph* LiveEngine::get_graph() const {
    return this->graph;
}

Logger* LiveEngine::get_logger() const {
    return this->logger;
}

void LiveEngine::add_streamer(std::unique_ptr<LiveStreamer> streamer) {
    if (!streamer) {
        throw std::runtime_error("LiveEngine::add_streamer received null streamer");
    }
    this->max_streamer_id += 1;
    streamer->set_id(this->max_streamer_id);
    this->streamers[this->max_streamer_id] = std::move(streamer);
}

const std::map<size_t, std::unique_ptr<LiveStreamer>>& LiveEngine::get_streamers() const {
    return this->streamers;
}

LiveStreamer* LiveEngine::get_streamer(const size_t& streamer_id) const {
    const auto it = this->streamers.find(streamer_id);
    if (it == this->streamers.end() || !it->second) {
        return nullptr;
    }
    return it->second.get();
}

void LiveEngine::register_source(Producer* source_node) {
    if (!source_node) {
        throw std::runtime_error("LiveEngine::register_source received null source node");
    }
    if (source_node->get_node_id() == 0) {
        throw std::runtime_error(fmt::format(
            "LiveEngine::register_source node {} is not attached to graph",
            source_node->get_name()
        ));
    }

    if (auto* market = dynamic_cast<Market*>(source_node)) {
        this->register_market_source(market);
        return;
    }

    throw std::runtime_error(fmt::format(
        "LiveEngine::register_source unsupported producer type: {}",
        source_node->get_name()
    ));
}

void LiveEngine::register_market_source(Market* market) {
    if (!market) {
        throw std::runtime_error("LiveEngine::register_market_source received null market");
    }

    for (auto& kv : this->streamers) {
        if (!kv.second) {
            continue;
        }
        auto* market_streamer = dynamic_cast<MarketStreamer*>(kv.second.get());
        if (!market_streamer) {
            continue;
        }
        if (market_streamer->get_instrument() == market->get_instrument()
            && market_streamer->get_exchange() == market->get_exchange()) {
            market_streamer->set_order_book_source_node_id(market->get_node_id());
            if (this->logger) {
                this->logger->log_info(
                    "LiveEngine",
                    fmt::format(
                        "Reusing existing live streamer {} -> source_id={}",
                        kv.second->get_name(),
                        market->get_node_id()
                    )
                );
            }
            return;
        }
    }

    std::string exchange = market->get_exchange();
    std::transform(exchange.begin(), exchange.end(), exchange.begin(), [](const unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    const SaphirManager saphir;
    const size_t ring_capacity = saphir.get_ring_capacity();
    const size_t max_update_batch_size = saphir.get_max_update_batch_size();

    std::unique_ptr<LiveStreamer> new_streamer;
    if (exchange == "bitmex") {
        auto bitmex_streamer = std::make_unique<BitmexLiveOrderBookStreamer>(
            market->get_instrument(),
            ring_capacity,
            max_update_batch_size
        );
        bitmex_streamer->set_order_book_source_node_id(market->get_node_id());
        new_streamer = std::move(bitmex_streamer);
    } else if (exchange == "binance") {
        auto binance_streamer = std::make_unique<BinanceLiveOrderBookStreamer>(
            market->get_instrument(),
            ring_capacity,
            max_update_batch_size
        );
        binance_streamer->set_order_book_source_node_id(market->get_node_id());
        new_streamer = std::move(binance_streamer);
    } else if (exchange == "deribit") {
        auto deribit_streamer = std::make_unique<DeribitLiveOrderBookStreamer>(
            market->get_instrument(),
            ring_capacity,
            max_update_batch_size
        );
        deribit_streamer->set_order_book_source_node_id(market->get_node_id());
        new_streamer = std::move(deribit_streamer);
    } else if (exchange == "okx") {
        auto okx_streamer = std::make_unique<OkxLiveOrderBookStreamer>(
            market->get_instrument(),
            ring_capacity,
            max_update_batch_size
        );
        okx_streamer->set_order_book_source_node_id(market->get_node_id());
        new_streamer = std::move(okx_streamer);
    } else {
        throw std::runtime_error(fmt::format(
            "LiveEngine::register_market_source unsupported exchange '{}' for instrument '{}'",
            market->get_exchange(),
            market->get_instrument()
        ));
    }

    if (this->logger) {
        this->logger->log_info(
            "LiveEngine",
            fmt::format(
                "Creating live streamer {} for source {} (node_id={})",
                new_streamer->get_name(),
                market->get_name(),
                market->get_node_id()
            )
        );
    }
    this->add_streamer(std::move(new_streamer));
}

void LiveEngine::build_streamer_container() {
    if (!this->graph) {
        throw std::runtime_error("LiveEngine::build_streamer_container graph is null");
    }
    if (this->graph->get_producer_container().empty()) {
        if (this->logger) {
            this->logger->log_error("LiveEngine", "Cannot build live streamers: producer container is empty");
        }
        throw std::runtime_error("LiveEngine::build_streamer_container producer container is empty");
    }

    for (const auto& source : this->graph->get_producer_container()) {
        this->register_source(source.second);
    }
}

void LiveEngine::initialize() {
    if (!this->graph) {
        throw std::runtime_error("LiveEngine::initialize graph is null");
    }

    if (this->logger) {
        this->logger->log_info("LiveEngine", "Initializing Live engine");
        this->logger->log_info("LiveEngine", "Resolving update path in graph");
    }
    this->graph->resolve_update_path();

    if (this->logger) {
        this->logger->log_info("LiveEngine", "Building live streamers");
    }
    this->build_streamer_container();
    if (this->logger) {
        this->logger->flush();
    }
}

void LiveEngine::run() {
    this->initialize();
    this->start();
    this->run_consumer_loop();
    this->join();
}

void LiveEngine::start() {
    if (!this->graph) {
        throw std::runtime_error("LiveEngine::start graph is null");
    }
    if (this->streamers.empty()) {
        throw std::runtime_error("LiveEngine::start called with empty streamer container");
    }

    bool expected = false;
    if (!this->running.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return;
    }

    for (auto& streamer : this->streamers) {
        if (streamer.second) {
            streamer.second->start();
        }
    }
}

void LiveEngine::stop() {
    this->running.store(false, std::memory_order_release);
    for (auto& streamer : this->streamers) {
        if (streamer.second) {
            streamer.second->stop();
        }
    }
}

void LiveEngine::join() {
    for (auto& streamer : this->streamers) {
        if (streamer.second) {
            streamer.second->join();
        }
    }
}

bool LiveEngine::is_running() const {
    return this->running.load(std::memory_order_acquire);
}

void LiveEngine::run_consumer_loop() {
    struct HeapEntry {
        int64_t reception_timestamp;
        size_t streamer_id;
    };
    struct HeapEntryCompare {
        bool operator()(const HeapEntry& lhs, const HeapEntry& rhs) const {
            if (lhs.reception_timestamp != rhs.reception_timestamp) {
                return lhs.reception_timestamp > rhs.reception_timestamp;
            }
            return lhs.streamer_id > rhs.streamer_id;
        }
    };

    std::vector<LiveStreamer*> streamer_by_id(this->max_streamer_id + 1, nullptr);
    for (const auto& kv : this->streamers) {
        if (!kv.second) {
            throw std::runtime_error(fmt::format(
                "LiveEngine::run_consumer_loop null streamer for streamer_id={}",
                kv.first
            ));
        }
        streamer_by_id[kv.first] = kv.second.get();
    }

    std::priority_queue<HeapEntry, std::vector<HeapEntry>, HeapEntryCompare> ready_heap;
    std::vector<uint8_t> streamer_in_heap(this->max_streamer_id + 1, 0);

    auto enqueue_streamer_head = [&](const size_t streamer_id) {
        if (streamer_in_heap[streamer_id] != 0) {
            return;
        }
        LiveStreamer* streamer = streamer_by_id[streamer_id];
        const Event* head_event = streamer->peek_event();
        if (!head_event) {
            return;
        }
        ready_heap.push(HeapEntry{head_event->get_reception_timestamp(), streamer_id});
        streamer_in_heap[streamer_id] = 1;
    };

    auto rescan_streamers = [&]() {
        for (size_t streamer_id = 1; streamer_id < streamer_by_id.size(); ++streamer_id) {
            enqueue_streamer_head(streamer_id);
        }
    };

    rescan_streamers();

    size_t idle_cycles = 0;
    size_t processed_events = 0;
    static constexpr size_t kRescanEveryNEvents = 64;

    while (this->running.load(std::memory_order_acquire)) {
        if (ready_heap.empty() || (processed_events % kRescanEveryNEvents) == 0) {
            rescan_streamers();
        }

        if (ready_heap.empty()) {
            idle_cycles += 1;
            if (idle_cycles < 1024) {
                std::this_thread::yield();
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds(50));
                idle_cycles = 0;
            }
            continue;
        }

        const HeapEntry candidate = ready_heap.top();
        ready_heap.pop();

        streamer_in_heap[candidate.streamer_id] = 0;
        LiveStreamer* selected_streamer = streamer_by_id[candidate.streamer_id];

        const Event* head_event = selected_streamer->peek_event();
        if (!head_event) {
            continue;
        }

        const int64_t current_head_ts = head_event->get_reception_timestamp();
        if (current_head_ts != candidate.reception_timestamp) {
            ready_heap.push(HeapEntry{current_head_ts, candidate.streamer_id});
            streamer_in_heap[candidate.streamer_id] = 1;
            continue;
        }

        idle_cycles = 0;
        LiveStreamer::EventPtr event;
        if (!selected_streamer->pop_event(event)) {
            continue;
        }

        const Event* next_head_event = selected_streamer->peek_event();
        if (next_head_event) {
            ready_heap.push(HeapEntry{next_head_event->get_reception_timestamp(), candidate.streamer_id});
            streamer_in_heap[candidate.streamer_id] = 1;
        }

        processed_events += 1;
        this->process_event(std::move(event));
    }
}

void LiveEngine::process_event(LiveStreamer::EventPtr event) {
    const int source_id = event->get_source_id_trigger();
    Producer* producer = this->graph->get_producer_container().at(source_id);
    producer->on_event(event.get());
    this->graph->update(source_id);
}
