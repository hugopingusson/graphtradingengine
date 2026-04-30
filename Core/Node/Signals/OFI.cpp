//
// Created by hugo on 29/04/2026.
//

#include "OFI.h"

#include <limits>

OFI::OFI():MarketSignal() {}
OFI::OFI(const string &instrument, const string &exchange):MarketSignal(fmt::format("OFI({};{})",instrument,exchange),instrument,exchange,3) {}

bool OFI::recompute() {
    const double nan = std::numeric_limits<double>::quiet_NaN();

    for (size_t index = 0; index < this->get_slot_count(); ++index) {
        if (this->is_active(index)) {
            this->set_slot_output(index, nan, false);
        } else {
            this->set_slot_output(index, nan, true);
        }
    }

    this->refresh_global_slot_validity();
    const bool previous_validity = this->valid;
    this->valid = this->are_slots_valid();
    return previous_validity != this->valid;
}
