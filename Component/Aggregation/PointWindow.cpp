//
// Created by hugo on 29/04/26.
//

#include "PointWindow.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
double quiet_nan() {
    return std::numeric_limits<double>::quiet_NaN();
}
}

PointWindow::PointWindow(const size_t& capacity)
    : capacity_(capacity),
      values_(),
      sorted_values_(),
      sum_(0.0),
      sum_squares_(0.0) {
    if (this->capacity_ == 0) {
        throw std::invalid_argument("PointWindow capacity must be > 0");
    }
}

void PointWindow::insert_value(const double& value) {
    this->values_.push_back(value);
    this->sorted_values_.insert(value);
    this->sum_ += value;
    this->sum_squares_ += value * value;
}

void PointWindow::erase_value(const double& value) {
    const auto sorted_it = this->sorted_values_.find(value);
    if (sorted_it != this->sorted_values_.end()) {
        this->sorted_values_.erase(sorted_it);
    }
    this->sum_ -= value;
    this->sum_squares_ -= value * value;
}

void PointWindow::enqueue(const double& value) {
    this->insert_value(value);
}

bool PointWindow::dequeue() {
    if (this->values_.empty()) {
        return false;
    }

    const double oldest = this->values_.front();
    this->values_.pop_front();
    this->erase_value(oldest);
    return true;
}

void PointWindow::add(const double& value) {
    this->enqueue(value);
    while (this->values_.size() > this->capacity_) {
        this->dequeue();
    }
}

size_t PointWindow::size() const {
    return this->values_.size();
}

size_t PointWindow::capacity() const {
    return this->capacity_;
}

bool PointWindow::empty() const {
    return this->values_.empty();
}

void PointWindow::clear() {
    this->values_.clear();
    this->sorted_values_.clear();
    this->sum_ = 0.0;
    this->sum_squares_ = 0.0;
}

double PointWindow::first() const {
    if (this->values_.empty()) {
        return quiet_nan();
    }
    return this->values_.front();
}

double PointWindow::last() const {
    if (this->values_.empty()) {
        return quiet_nan();
    }
    return this->values_.back();
}

double PointWindow::max() const {
    if (this->sorted_values_.empty()) {
        return quiet_nan();
    }
    return *this->sorted_values_.rbegin();
}

double PointWindow::min() const {
    if (this->sorted_values_.empty()) {
        return quiet_nan();
    }
    return *this->sorted_values_.begin();
}

double PointWindow::average() const {
    if (this->values_.empty()) {
        return quiet_nan();
    }
    return this->sum_ / static_cast<double>(this->values_.size());
}

double PointWindow::std() const {
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
