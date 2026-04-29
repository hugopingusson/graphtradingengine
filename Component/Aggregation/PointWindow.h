//
// Created by hugo on 29/04/26.
//

#ifndef POINTWINDOW_H
#define POINTWINDOW_H

#include <cstddef>
#include <deque>
#include <set>

class PointWindow {
public:
    explicit PointWindow(const size_t& capacity);

    void enqueue(const double& value);
    bool dequeue();
    void add(const double& value);

    [[nodiscard]] size_t size() const;
    [[nodiscard]] size_t capacity() const;
    [[nodiscard]] bool empty() const;
    void clear();

    [[nodiscard]] double first() const;
    [[nodiscard]] double last() const;
    [[nodiscard]] double max() const;
    [[nodiscard]] double min() const;
    [[nodiscard]] double average() const;
    [[nodiscard]] double std() const;

private:
    void insert_value(const double& value);
    void erase_value(const double& value);

    size_t capacity_;
    std::deque<double> values_;
    std::multiset<double> sorted_values_;
    double sum_;
    double sum_squares_;
};

#endif //POINTWINDOW_H
