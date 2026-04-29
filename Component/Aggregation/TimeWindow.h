//
// Created by hugo on 29/04/26.
//

#ifndef TIMEWINDOW_H
#define TIMEWINDOW_H

#include <cstdint>
#include <cstddef>
#include <deque>
#include <set>

class TimeWindow {
public:
    explicit TimeWindow(const int64_t& horizon_nanoseconds);

    void enqueue(const double& value, const int64_t& timestamp_nanoseconds);
    void enqueue_now(const double& value);
    bool dequeue();

    void add(const double& value, const int64_t& timestamp_nanoseconds);
    void add_now(const double& value);

    [[nodiscard]] size_t size() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] int64_t get_horizon_nanoseconds() const;
    void clear();

    [[nodiscard]] double first() const;
    [[nodiscard]] double last() const;
    [[nodiscard]] double max() const;
    [[nodiscard]] double min() const;
    [[nodiscard]] double average() const;
    [[nodiscard]] double std() const;

private:
    struct TimedValue {
        int64_t timestamp_nanoseconds;
        double value;
    };

    void insert_value(const double& value, const int64_t& timestamp_nanoseconds);
    void erase_value(const double& value);
    void evict_stale(const int64_t& reference_timestamp_nanoseconds);

    int64_t horizon_nanoseconds_;
    std::deque<TimedValue> values_;
    std::multiset<double> sorted_values_;
    double sum_;
    double sum_squares_;
};

#endif //TIMEWINDOW_H
