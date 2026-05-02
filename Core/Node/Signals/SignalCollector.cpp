//
// Created by hugo on 30/04/26.
//

#include "SignalCollector.h"

#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

#include "../../Graph/Graph.h"

namespace {
double quiet_nan() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool are_equal_with_nan(const double& lhs, const double& rhs) {
    if (std::isnan(lhs) && std::isnan(rhs)) {
        return true;
    }
    return lhs == rhs;
}

bool are_vectors_equal_with_nan(const vector<double>& lhs,
                                const vector<double>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }

    for (size_t index = 0; index < lhs.size(); ++index) {
        if (!are_equal_with_nan(lhs[index], rhs[index])) {
            return false;
        }
    }

    return true;
}
}

SignalCollector::SignalCollector()
    : Consumer(),
      signal_bindings(),
      total_slot_count(0),
      features(),
      feature_valid_mask(),
      all_valid(true) {
    this->valid = true;
}

SignalCollector::SignalCollector(const string& name)
    : SignalCollector() {
    this->name = name;
}

void SignalCollector::add_signal(Graph& graph,
                                 Consumer& signal_owner,
                                 SignalSlots& signal_slots) {
    for (const SignalBinding& binding : this->signal_bindings) {
        if (binding.owner == &signal_owner && binding.slots == &signal_slots) {
            throw std::invalid_argument("SignalCollector::add_signal duplicate binding");
        }
    }

    graph.add_edge(&signal_owner, this);
    this->signal_bindings.push_back(SignalBinding{&signal_owner, &signal_slots});
    this->total_slot_count += signal_slots.get_slot_count();
    this->mark_scheduled();
}

bool SignalCollector::update() {
    vector<double> new_features(this->total_slot_count, quiet_nan());
    vector<uint8_t> new_feature_valid_mask(this->total_slot_count, 0);

    size_t offset = 0;
    bool new_all_valid = true;

    for (const SignalBinding& binding : this->signal_bindings) {
        const SignalSlots& slots = *binding.slots;
        const size_t slot_count = slots.get_slot_count();
        const vector<double>& slot_values = slots.get_values();
        const vector<uint8_t>& slot_valid_mask = slots.get_valid_mask();

        for (size_t slot_index = 0; slot_index < slot_count; ++slot_index) {
            const size_t feature_index = offset + slot_index;
            new_features[feature_index] = slot_values[slot_index];
            new_feature_valid_mask[feature_index] = slot_valid_mask[slot_index];
            new_all_valid = new_all_valid && (slot_valid_mask[slot_index] != 0);
        }

        offset += slot_count;
    }

    const bool feature_values_changed = !are_vectors_equal_with_nan(this->features, new_features);
    const bool feature_validity_changed = this->feature_valid_mask != new_feature_valid_mask;
    const bool all_valid_changed = this->all_valid != new_all_valid;

    this->features = std::move(new_features);
    this->feature_valid_mask = std::move(new_feature_valid_mask);
    this->all_valid = new_all_valid;

    const bool previous_validity = this->valid;
    this->valid = this->all_valid;
    const bool node_validity_changed = previous_validity != this->valid;

    return feature_values_changed || feature_validity_changed || all_valid_changed || node_validity_changed;
}

size_t SignalCollector::get_signal_count() const {
    return this->signal_bindings.size();
}

size_t SignalCollector::get_feature_count() const {
    return this->features.size();
}

const vector<double>& SignalCollector::get_features() const {
    return this->features;
}

const vector<uint8_t>& SignalCollector::get_feature_valid_mask() const {
    return this->feature_valid_mask;
}

bool SignalCollector::is_all_valid() const {
    return this->all_valid;
}
