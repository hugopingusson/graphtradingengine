//
// Created by hugo on 03/04/26.
//

#ifndef LIVESTREAMER_H
#define LIVESTREAMER_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "Streamer.h"

template <typename T>
class SpscRingBuffer {
public:
    explicit SpscRingBuffer(size_t requested_capacity)
        : capacity_(round_up_power_of_two(requested_capacity < 2 ? 2 : requested_capacity)),
          mask_(capacity_ - 1),
          buffer_(capacity_),
          head_(0),
          tail_(0) {}

    bool push(T item) {
        const size_t head = head_.load(std::memory_order_relaxed);
        const size_t tail = tail_.load(std::memory_order_acquire);
        if (head - tail >= capacity_) {
            return false;
        }

        buffer_[head & mask_] = std::move(item);
        head_.store(head + 1, std::memory_order_release);
        return true;
    }

    bool pop(T& out) {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t head = head_.load(std::memory_order_acquire);
        if (tail == head) {
            return false;
        }

        out = std::move(buffer_[tail & mask_]);
        tail_.store(tail + 1, std::memory_order_release);
        return true;
    }

    bool peek(const T*& out) const {
        const size_t tail = tail_.load(std::memory_order_relaxed);
        const size_t head = head_.load(std::memory_order_acquire);
        if (tail == head) {
            out = nullptr;
            return false;
        }

        out = &buffer_[tail & mask_];
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    size_t size() const {
        const size_t head = head_.load(std::memory_order_acquire);
        const size_t tail = tail_.load(std::memory_order_acquire);
        return head - tail;
    }

    size_t capacity() const {
        return capacity_;
    }

private:
    static size_t round_up_power_of_two(size_t value) {
        size_t power = 1;
        while (power < value) {
            power <<= 1;
        }
        return power;
    }

    const size_t capacity_;
    const size_t mask_;
    mutable std::vector<T> buffer_;
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

class LiveStreamer : public Streamer {
public:
    using EventPtr = std::unique_ptr<Event>;

    explicit LiveStreamer(std::string name, size_t ring_capacity = 1u << 16);
    ~LiveStreamer() override;

    std::string get_name() override;
    void set_id(const size_t& id);
    size_t get_id() const;

    bool start();
    void stop();
    void join();
    bool is_running() const;
    bool is_stop_requested() const;

    bool pop_event(EventPtr& out_event);
    const Event* peek_event() const;
    bool has_events() const;
    size_t ring_size() const;
    size_t ring_capacity() const;

    uint64_t get_pushed_events() const;
    uint64_t get_dropped_events() const;

protected:
    bool push_event(EventPtr event);
    virtual void run_loop() = 0;

private:
    size_t id_;
    std::string name_;
    SpscRingBuffer<EventPtr> ring_;
    std::thread worker_;
    std::atomic<bool> running_;
    std::atomic<bool> stop_requested_;
    std::atomic<uint64_t> pushed_events_;
    std::atomic<uint64_t> dropped_events_;
};

class LiveOrderBookStreamer : public LiveStreamer,
                              virtual public MarketStreamer {
public:
    LiveOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity = 1u << 16);
    ~LiveOrderBookStreamer() override = default;

protected:
    bool emit_mbp_event(const int64_t& reception_timestamp,
                        const int& source_id_trigger,
                        const SnapshotMessage& message,
                        const Location& location = Location::UNKNOWN,
                        const Listener& listener = Listener::PRODUCTION);

    bool emit_mbo_event(const int64_t& reception_timestamp,
                        const int& source_id_trigger,
                        const OrderMessage& message,
                        const Location& location = Location::UNKNOWN,
                        const Listener& listener = Listener::PRODUCTION);

    bool emit_update_event(const int64_t& reception_timestamp,
                           const int& source_id_trigger,
                           const UpdateMessage& message,
                           const Location& location = Location::UNKNOWN,
                           const Listener& listener = Listener::PRODUCTION);
};

class LiveBootstrappedOrderBookStreamer : public LiveOrderBookStreamer {
public:
    LiveBootstrappedOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity = 1u << 16);
    ~LiveBootstrappedOrderBookStreamer() override = default;

protected:
    bool is_bootstrapped() const;
    void mark_bootstrapped();
    void reset_bootstrap();

private:
    std::atomic<bool> bootstrapped_;
};

class LiveOrderDeltaOrderBookStreamer : public LiveBootstrappedOrderBookStreamer {
public:
    LiveOrderDeltaOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity = 1u << 16);
    ~LiveOrderDeltaOrderBookStreamer() override = default;
};

class LiveUpdateDeltaOrderBookStreamer : public LiveBootstrappedOrderBookStreamer {
public:
    LiveUpdateDeltaOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity = 1u << 16);
    ~LiveUpdateDeltaOrderBookStreamer() override = default;
};

class LiveSnapshotOrderBookStreamer : public LiveOrderBookStreamer {
public:
    LiveSnapshotOrderBookStreamer(const std::string& name, const std::string& instrument, const std::string& exchange, size_t ring_capacity = 1u << 16);
    ~LiveSnapshotOrderBookStreamer() override = default;
};

#endif //LIVESTREAMER_H
