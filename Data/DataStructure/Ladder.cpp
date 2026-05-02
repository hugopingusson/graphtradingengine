//
// Created by hugo on 30/04/26.
//

#include "Ladder.h"

#include <algorithm>
#include <stdexcept>

Ladder::Ladder()
    : levels_(),
      effective_depth_(0) {}

std::size_t Ladder::effective_depth() const {
    return this->effective_depth_;
}

bool Ladder::empty() const {
    return this->effective_depth_ == 0;
}

void Ladder::reset() {
    this->levels_.fill(BookLevel{});
    this->effective_depth_ = 0;
}

const BookLevel& Ladder::at(const std::size_t& index) const {
    if (index >= kBookLevels) {
        throw std::out_of_range("Ladder::at index out of range");
    }
    return this->levels_[index];
}

BookLevel& Ladder::at(const std::size_t& index) {
    if (index >= kBookLevels) {
        throw std::out_of_range("Ladder::at index out of range");
    }
    return this->levels_[index];
}

const BookLevel& Ladder::operator[](const std::size_t& index) const {
    return this->at(index);
}

BookLevel& Ladder::operator[](const std::size_t& index) {
    return this->at(index);
}

bool Ladder::append(const BookLevel& level) {
    if (this->effective_depth_ >= kBookLevels) {
        return false;
    }

    this->levels_[this->effective_depth_++] = level;
    return true;
}

void Ladder::erase(const std::size_t& index) {
    if (index >= this->effective_depth_) {
        return;
    }

    for (std::size_t i = index + 1; i < this->effective_depth_; ++i) {
        this->levels_[i - 1] = this->levels_[i];
    }

    --this->effective_depth_;
    this->levels_[this->effective_depth_] = BookLevel{};
}

void Ladder::insert_at(const std::size_t& position,
                       const BookLevel& level) {
    if (position >= kBookLevels) {
        return;
    }

    const std::size_t insert_pos = std::min<std::size_t>(position, this->effective_depth_);
    const std::size_t new_depth = std::min<std::size_t>(kBookLevels, this->effective_depth_ + 1);

    for (std::size_t i = new_depth; i > insert_pos + 1; --i) {
        this->levels_[i - 1] = this->levels_[i - 2];
    }

    this->levels_[insert_pos] = level;
    this->effective_depth_ = new_depth;
}
