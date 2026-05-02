//
// Created by hugo on 30/04/26.
//

#ifndef LADDER_H
#define LADDER_H

#include <array>
#include <cstddef>
#include <string>

#include "DataStructure.h"

class Ladder {
public:
    Ladder();

    [[nodiscard]] std::size_t effective_depth() const;
    [[nodiscard]] bool empty() const;

    void reset();

    [[nodiscard]] const BookLevel& at(const std::size_t& index) const;
    BookLevel& at(const std::size_t& index);

    [[nodiscard]] const BookLevel& operator[](const std::size_t& index) const;
    BookLevel& operator[](const std::size_t& index);

    bool append(const BookLevel& level);
    void erase(const std::size_t& index);
    void insert_at(const std::size_t& position,
                   const BookLevel& level);

private:
    std::array<BookLevel, kBookLevels> levels_;
    std::size_t effective_depth_;
};

#endif //LADDER_H
