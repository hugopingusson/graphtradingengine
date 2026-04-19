//
// Created by hugo on 03/04/26.
//

#include "LiveStreamer.h"

#include <algorithm>

LiveStreamer::LiveStreamer(std::string name, size_t ring_capacity)
    : id_(0),
      name_(std::move(name)),
      ring_(ring_capacity),
      worker_(),
      running_(false),
      stop_requested_(false),
      reconnect_requested_(false),
      desynced_(false),
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
    reconnect_requested_.store(false, std::memory_order_release);
    desynced_.store(false, std::memory_order_release);
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
        this->mark_desynced();
        this->on_ring_overflow();
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

bool LiveStreamer::is_desynced() const {
    return desynced_.load(std::memory_order_acquire);
}

void LiveStreamer::request_reconnect() {
    reconnect_requested_.store(true, std::memory_order_release);
}

bool LiveStreamer::consume_reconnect_request() {
    return reconnect_requested_.exchange(false, std::memory_order_acq_rel);
}

void LiveStreamer::mark_synced() {
    desynced_.store(false, std::memory_order_release);
}

void LiveStreamer::mark_desynced() {
    desynced_.store(true, std::memory_order_release);
}

void LiveStreamer::on_ring_overflow() {
    this->request_reconnect();
}

LiveOrderBookStreamer::LiveOrderBookStreamer(const std::string& name,
                                             const std::string& instrument,
                                             const std::string& exchange,
                                             size_t ring_capacity,
                                             size_t max_update_batch_size)
    : MarketStreamer(instrument, exchange),
      LiveStreamer(name, ring_capacity),
      max_update_batch_size_(max_update_batch_size == 0 ? 1 : max_update_batch_size) {}

bool LiveOrderBookStreamer::emit_mbp_event(const int64_t& reception_timestamp,
                                           const int& source_id_trigger,
                                           const SnapshotMessage& message,
                                           const Location& location,
                                           const Listener& listener) {
    const bool pushed = this->push_event(std::make_unique<SnapshotEvent>(
        this->get_instrument(),
        reception_timestamp,
        source_id_trigger,
        message,
        location,
        listener
    ));
    if (pushed) {
        this->mark_synced();
    }
    return pushed;
}

bool LiveOrderBookStreamer::emit_mbo_event(const int64_t& reception_timestamp,
                                           const int& source_id_trigger,
                                           const OrderMessage& message,
                                           const Location& location,
                                           const Listener& listener) {
    if (this->is_desynced()) {
        return false;
    }
    return this->push_event(std::make_unique<OrderEvent>(
        this->get_instrument(),
        reception_timestamp,
        source_id_trigger,
        message,
        location,
        listener
    ));
}

bool LiveOrderBookStreamer::emit_update_event(const int64_t& reception_timestamp,
                                              const int& source_id_trigger,
                                              const UpdateMessage& message,
                                              const Location& location,
                                              const Listener& listener) {
    if (this->is_desynced()) {
        return false;
    }
    return this->push_event(std::make_unique<UpdateEvent>(
        this->get_instrument(),
        reception_timestamp,
        source_id_trigger,
        message,
        location,
        listener
    ));
}

bool LiveOrderBookStreamer::emit_update_batch_event(const int64_t& reception_timestamp,
                                                    const int& source_id_trigger,
                                                    const std::vector<UpdateMessage>& messages,
                                                    const Location& location,
                                                    const Listener& listener) {
    if (this->is_desynced()) {
        return false;
    }
    if (messages.empty()) {
        return false;
    }

    const size_t chunk_size = this->max_update_batch_size_;
    size_t offset = 0;
    while (offset < messages.size()) {
        const size_t count = std::min(chunk_size, messages.size() - offset);
        std::vector<UpdateMessage> chunk(messages.begin() + static_cast<std::ptrdiff_t>(offset),
                                         messages.begin() + static_cast<std::ptrdiff_t>(offset + count));
        if (!this->push_event(std::make_unique<UpdateBatchEvent>(
            this->get_instrument(),
            reception_timestamp,
            source_id_trigger,
            chunk,
            location,
            listener
        ))) {
            return false;
        }
        offset += count;
    }
    return true;
}

size_t LiveOrderBookStreamer::get_max_update_batch_size() const {
    return this->max_update_batch_size_;
}

LiveBootstrappedOrderBookStreamer::LiveBootstrappedOrderBookStreamer(const std::string& name,
                                                                     const std::string& instrument,
                                                                     const std::string& exchange,
                                                                     size_t ring_capacity,
                                                                     size_t max_update_batch_size)
    : LiveOrderBookStreamer(name, instrument, exchange, ring_capacity, max_update_batch_size),
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

void LiveBootstrappedOrderBookStreamer::on_ring_overflow() {
    this->reset_bootstrap();
    LiveStreamer::on_ring_overflow();
}

LiveOrderDeltaOrderBookStreamer::LiveOrderDeltaOrderBookStreamer(const std::string& name,
                                                                 const std::string& instrument,
                                                                 const std::string& exchange,
                                                                 size_t ring_capacity,
                                                                 size_t max_update_batch_size)
    : LiveBootstrappedOrderBookStreamer(name, instrument, exchange, ring_capacity, max_update_batch_size) {}

LiveUpdateDeltaOrderBookStreamer::LiveUpdateDeltaOrderBookStreamer(const std::string& name,
                                                                   const std::string& instrument,
                                                                   const std::string& exchange,
                                                                   size_t ring_capacity,
                                                                   size_t max_update_batch_size)
    : LiveBootstrappedOrderBookStreamer(name, instrument, exchange, ring_capacity, max_update_batch_size) {}

LiveSnapshotOrderBookStreamer::LiveSnapshotOrderBookStreamer(const std::string& name,
                                                             const std::string& instrument,
                                                             const std::string& exchange,
                                                             size_t ring_capacity,
                                                             size_t max_update_batch_size)
    : LiveOrderBookStreamer(name, instrument, exchange, ring_capacity, max_update_batch_size) {}
