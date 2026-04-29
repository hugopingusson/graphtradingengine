//
// Created by hugo on 29/04/26.
//

#include "TimeWindow.h"

#include "Helper/TimeHelper.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
double quiet_nan() {
    return std::numeric_limits<double>::quiet_NaN();
}
}

TimeWindow::TimeWindow(const int64_t& horizon_nanoseconds)
    : horizon_nanoseconds_(horizon_nanoseconds),
      values_(),
      sorted_values_(),
      sum_(0.0),
      sum_squares_(0.0) {
    if (this->horizon_nanoseconds_ < 0) {
        throw std::invalid_argument("TimeWindow horizon must be >= 0");
    }
}

void TimeWindow::insert_value(const double& value, const int64_t& timestamp_nanoseconds) {
    this->values_.push_back(TimedValue{timestamp_nanoseconds, value});
    this->sorted_values_.insert(value);
    this->sum_ += value;
    this->sum_squares_ += value * value;
}

void TimeWindow::erase_value(const double& value) {
    const auto sorted_it = this->sorted_values_.find(value);
    if (sorted_it != this->sorted_values_.end()) {
        this->sorted_values_.erase(sorted_it);
    }
    this->sum_ -= value;
    this->sum_squares_ -= value * value;
}

void TimeWindow::evict_stale(const int64_t& reference_timestamp_nanoseconds) {
    const int64_t lower_bound = reference_timestamp_nanoseconds - this->horizon_nanoseconds_;
    while (!this->values_.empty() && this->values_.front().timestamp_nanoseconds < lower_bound) {
        this->dequeue();
    }
}

void TimeWindow::enqueue(const double& value, const int64_t& timestamp_nanoseconds) {
    this->insert_value(value, timestamp_nanoseconds);
}

void TimeWindow::enqueue_now(const double& value) {
    this->enqueue(value, Timestamp::now_unix(Resolution::nanoseconds));
}

bool TimeWindow::dequeue() {
    if (this->values_.empty()) {
        return false;
    }

    const double oldest = this->values_.front().value;
    this->values_.pop_front();
    this->erase_value(oldest);
    return true;
}

void TimeWindow::add(const double& value, const int64_t& timestamp_nanoseconds) {
    this->enqueue(value, timestamp_nanoseconds);
    this->evict_stale(timestamp_nanoseconds);
}

void TimeWindow::add_now(const double& value) {
    this->add(value, Timestamp::now_unix(Resolution::nanoseconds));
}

size_t TimeWindow::size() const {
    return this->values_.size();
}

bool TimeWindow::empty() const {
    return this->values_.empty();
}

int64_t TimeWindow::get_horizon_nanoseconds() const {
    return this->horizon_nanoseconds_;
}

void TimeWindow::clear() {
    this->values_.clear();
    this->sorted_values_.clear();
    this->sum_ = 0.0;
    this->sum_squares_ = 0.0;
}

double TimeWindow::first() const {
    if (this->values_.empty()) {
        return quiet_nan();
    }
    return this->values_.front().value;
}

double TimeWindow::last() const {
    if (this->values_.empty()) {
        return quiet_nan();
    }
    return this->values_.back().value;
}

double TimeWindow::max() const {
    if (this->sorted_values_.empty()) {
        return quiet_nan();
    }
    return *this->sorted_values_.rbegin();
}

double TimeWindow::min() const {
    if (this->sorted_values_.empty()) {
        return quiet_nan();
    }
    return *this->sorted_values_.begin();
}

double TimeWindow::average() const {
    if (this->values_.empty()) {
        return quiet_nan();
    }
    return this->sum_ / static_cast<double>(this->values_.size());
}

double TimeWindow::std() const {
    const size_t n = this->values_.size();
    if (n < 2) {
        return quiet_nan();
    }

    const double n_as_double = static_cast<double>(n);
    double variance = (this->sum_squares_ - (this->sum_ * this->sum_ / n_as_double))
        / static_cast<double>(n - 1);
    if (variance < 0.0) {
        variance = 0.0;
    }
    return std::sqrt(variance);
}
