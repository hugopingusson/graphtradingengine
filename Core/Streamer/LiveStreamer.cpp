//
// Created by hugo on 03/04/26.
//

#include "LiveStreamer.h"

LiveStreamer::LiveStreamer(std::string name, size_t ring_capacity)
    : id_(0),
      name_(std::move(name)),
      ring_(ring_capacity),
      worker_(),
      running_(false),
      stop_requested_(false),
      pushed_events_(0),
      dropped_events_(0) {}

LiveStreamer::~LiveStreamer() {
    this->stop();
    this->join();
}

std::string LiveStreamer::get_name() {
    return this->name_;
}

void LiveStreamer::set_id(const size_t& id) {
    this->id_ = id;
}

size_t LiveStreamer::get_id() const {
    return this->id_;
}

bool LiveStreamer::start() {
    bool expected = false;
    if (!running_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        return false;
    }

    stop_requested_.store(false, std::memory_order_release);
    worker_ = std::thread([this]() {
        this->run_loop();
        running_.store(false, std::memory_order_release);
    });
    return true;
}

void LiveStreamer::stop() {
    stop_requested_.store(true, std::memory_order_release);
}

void LiveStreamer::join() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

bool LiveStreamer::is_running() const {
    return running_.load(std::memory_order_acquire);
}

bool LiveStreamer::is_stop_requested() const {
    return stop_requested_.load(std::memory_order_acquire);
}

bool LiveStreamer::push_event(EventPtr event) {
    if (!event) {
        return false;
    }

    if (!ring_.push(std::move(event))) {
        dropped_events_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    pushed_events_.fetch_add(1, std::memory_order_relaxed);
    return true;
}

bool LiveStreamer::pop_event(EventPtr& out_event) {
    return ring_.pop(out_event);
}

const Event* LiveStreamer::peek_event() const {
    const EventPtr* slot = nullptr;
    if (!ring_.peek(slot) || !slot || !(*slot)) {
        return nullptr;
    }
    return slot->get();
}

bool LiveStreamer::has_events() const {
    return !ring_.empty();
}

size_t LiveStreamer::ring_size() const {
    return ring_.size();
}

size_t LiveStreamer::ring_capacity() const {
    return ring_.capacity();
}

uint64_t LiveStreamer::get_pushed_events() const {
    return pushed_events_.load(std::memory_order_relaxed);
}

uint64_t LiveStreamer::get_dropped_events() const {
    return dropped_events_.load(std::memory_order_relaxed);
}

LiveOrderBookStreamer::LiveOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity)
    : MarketStreamer(instrument, exchange),
      LiveStreamer(name, ring_capacity) {}

bool LiveOrderBookStreamer::emit_mbp_event(const int64_t& capture_server_in_timestamp,
                                           const int64_t& streamer_in_timestamp,
                                           const int& source_id_trigger,
                                           const MarketByPriceMessage& message) {
    return this->push_event(std::make_unique<MBPEvent>(
        this->get_instrument(),
        capture_server_in_timestamp,
        streamer_in_timestamp,
        source_id_trigger,
        message
    ));
}

bool LiveOrderBookStreamer::emit_mbo_event(const int64_t& capture_server_in_timestamp,
                                           const int64_t& streamer_in_timestamp,
                                           const int& source_id_trigger,
                                           const MarketByOrderMessage& message) {
    return this->push_event(std::make_unique<MBOEvent>(
        this->get_instrument(),
        capture_server_in_timestamp,
        streamer_in_timestamp,
        source_id_trigger,
        message
    ));
}

bool LiveOrderBookStreamer::emit_update_event(const int64_t& capture_server_in_timestamp,
                                              const int64_t& streamer_in_timestamp,
                                              const int& source_id_trigger,
                                              const MarketUpdateMessage& message) {
    return this->push_event(std::make_unique<UpdateEvent>(
        this->get_instrument(),
        capture_server_in_timestamp,
        streamer_in_timestamp,
        source_id_trigger,
        message
    ));
}

LiveBootstrappedOrderBookStreamer::LiveBootstrappedOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity)
    : LiveOrderBookStreamer(name, instrument, exchange, ring_capacity),
      bootstrapped_(false) {}

bool LiveBootstrappedOrderBookStreamer::is_bootstrapped() const {
    return bootstrapped_.load(std::memory_order_acquire);
}

void LiveBootstrappedOrderBookStreamer::mark_bootstrapped() {
    bootstrapped_.store(true, std::memory_order_release);
}

void LiveBootstrappedOrderBookStreamer::reset_bootstrap() {
    bootstrapped_.store(false, std::memory_order_release);
}

LiveOrderDeltaOrderBookStreamer::LiveOrderDeltaOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity)
    : LiveBootstrappedOrderBookStreamer(name, instrument, exchange, ring_capacity) {}

LiveUpdateDeltaOrderBookStreamer::LiveUpdateDeltaOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity)
    : LiveBootstrappedOrderBookStreamer(name, instrument, exchange, ring_capacity) {}

LiveSnapshotOrderBookStreamer::LiveSnapshotOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity)
    : LiveOrderBookStreamer(name, instrument, exchange, ring_capacity) {}
