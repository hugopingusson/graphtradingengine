//
// Created by hugo on 28/04/26.
//

#include "Base.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "../Base/MarketNode.h"

namespace {
bool are_equal_with_nan(const double& lhs, const double& rhs) {
    if (std::isnan(lhs) && std::isnan(rhs)) {
        return true;
    }
    return lhs == rhs;
}

double quiet_nan() {
    return std::numeric_limits<double>::quiet_NaN();
}
}

SignalSlots::SignalSlots()
    : values(),
      active_mask(),
      valid_mask(),
      slots_valid(true) {}

SignalSlots::SignalSlots(const size_t& slot_count)
    : values(),
      active_mask(),
      valid_mask(),
      slots_valid(true) {
    this->values.assign(slot_count, quiet_nan());
    this->active_mask.assign(slot_count, 0);
    this->valid_mask.assign(slot_count, 1);
}

size_t SignalSlots::get_slot_count() const {
    return this->values.size();
}

bool SignalSlots::is_active(const size_t& index) const {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("SignalSlots::is_active index out of range");
    }
    return this->active_mask[index] != 0;
}

void SignalSlots::set_active(const size_t& index, const bool& active) {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("SignalSlots::set_active index out of range");
    }

    this->active_mask[index] = active ? 1 : 0;
    if (!active) {
        this->values[index] = quiet_nan();
        this->valid_mask[index] = 1;
    } else {
        this->values[index] = quiet_nan();
        this->valid_mask[index] = 0;
    }

    this->refresh_global_slot_validity();
}

void SignalSlots::active_all() {
    for (size_t index = 0; index < this->get_slot_count(); ++index) {
        this->active_mask[index] = 1;
        this->values[index] = quiet_nan();
        this->valid_mask[index] = 0;
    }
    this->refresh_global_slot_validity();
}

void SignalSlots::deactivate_all() {
    for (size_t index = 0; index < this->get_slot_count(); ++index) {
        this->active_mask[index] = 0;
        this->values[index] = quiet_nan();
        this->valid_mask[index] = 1;
    }
    this->refresh_global_slot_validity();
}

const vector<double>& SignalSlots::get_values() const {
    return this->values;
}

double SignalSlots::get_value(const size_t& index) const {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("SignalSlots::get_value index out of range");
    }
    return this->values[index];
}

const vector<uint8_t>& SignalSlots::get_active_mask() const {
    return this->active_mask;
}

const vector<uint8_t>& SignalSlots::get_valid_mask() const {
    return this->valid_mask;
}

bool SignalSlots::is_slot_valid(const size_t& index) const {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("SignalSlots::is_slot_valid index out of range");
    }
    return this->valid_mask[index] != 0;
}

bool SignalSlots::are_slots_valid() const {
    return this->slots_valid;
}

void SignalSlots::set_slot_output(const size_t& index,
                                  const double& value,
                                  const bool& valid) {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("SignalSlots::set_slot_output index out of range");
    }
    this->values[index] = value;
    this->valid_mask[index] = valid ? 1 : 0;
}

bool SignalSlots::refresh_global_slot_validity() {
    const bool previous_validity = this->slots_valid;
    bool new_validity = true;
    for (const uint8_t slot_validity : this->valid_mask) {
        new_validity = new_validity && (slot_validity != 0);
    }
    this->slots_valid = new_validity;
    return previous_validity != new_validity;
}

ParamStorage::ParamStorage()
    : hyperparameter_dimension(0),
      hyperparameter_vector_count(0),
      hyperparameter_space() {}

ParamStorage::ParamStorage(const size_t& hyperparameter_dimension,
                           const size_t& hyperparameter_vector_count)
    : hyperparameter_dimension(hyperparameter_dimension),
      hyperparameter_vector_count(hyperparameter_vector_count),
      hyperparameter_space() {
    if (this->hyperparameter_dimension == 0) {
        throw std::runtime_error("ParamStorage hyperparameter_dimension must be > 0");
    }

    this->hyperparameter_space.assign(
        this->hyperparameter_dimension * this->hyperparameter_vector_count,
        0.0
    );
}

size_t ParamStorage::get_hyperparameter_dimension() const {
    return this->hyperparameter_dimension;
}

size_t ParamStorage::get_hyperparameter_vector_count() const {
    return this->hyperparameter_vector_count;
}

const double* ParamStorage::get_hyperparameter_vector_data(const size_t& index) const {
    if (index >= this->hyperparameter_vector_count) {
        throw std::out_of_range("ParamStorage::get_hyperparameter_vector_data index out of range");
    }

    const size_t offset = index * this->hyperparameter_dimension;
    return this->hyperparameter_space.data() + static_cast<std::ptrdiff_t>(offset);
}

double* ParamStorage::get_hyperparameter_vector_data(const size_t& index) {
    if (index >= this->hyperparameter_vector_count) {
        throw std::out_of_range("ParamStorage::get_hyperparameter_vector_data index out of range");
    }

    const size_t offset = index * this->hyperparameter_dimension;
    return this->hyperparameter_space.data() + static_cast<std::ptrdiff_t>(offset);
}

void ParamStorage::set_hyperparameter_vector(const size_t& index,
                                             const vector<double>& hyperparameter_vector) {
    if (index >= this->hyperparameter_vector_count) {
        throw std::out_of_range("ParamStorage::set_hyperparameter_vector index out of range");
    }
    if (hyperparameter_vector.size() != this->hyperparameter_dimension) {
        throw std::invalid_argument("ParamStorage::set_hyperparameter_vector dimension mismatch");
    }

    double* destination = this->get_hyperparameter_vector_data(index);
    std::copy(hyperparameter_vector.begin(), hyperparameter_vector.end(), destination);
}

ParametrizedSignal::ParametrizedSignal()
    : SignalSlots(),
      name(),
      param_storage() {}

ParametrizedSignal::ParametrizedSignal(const string& name,
                                       const size_t& hyperparameter_dimension,
                                       const size_t& hyperparameter_vector_count)
    : SignalSlots(hyperparameter_vector_count),
      name(name),
      param_storage(hyperparameter_dimension, hyperparameter_vector_count) {}

const string& ParametrizedSignal::get_name() const {
    return this->name;
}

size_t ParametrizedSignal::get_hyperparameter_dimension() const {
    return this->param_storage.get_hyperparameter_dimension();
}

size_t ParametrizedSignal::get_hyperparameter_vector_count() const {
    return this->param_storage.get_hyperparameter_vector_count();
}

const double* ParametrizedSignal::get_hyperparameter_vector_data(const size_t& index) const {
    return this->param_storage.get_hyperparameter_vector_data(index);
}

double* ParametrizedSignal::get_hyperparameter_vector_data(const size_t& index) {
    return this->param_storage.get_hyperparameter_vector_data(index);
}

void ParametrizedSignal::set_hyperparameter_vector(const size_t& index,
                                                   const vector<double>& hyperparameter_vector) {
    this->param_storage.set_hyperparameter_vector(index, hyperparameter_vector);
}

MarketSignal::MarketSignal()
    : MarketConsumer(),
      SignalSlots() {}

MarketSignal::MarketSignal(const string& name,
                           const string& instrument,
                           const string& exchange,
                           const size_t& slot_count)
    : Node(),
      MarketConsumer(instrument, exchange),
      SignalSlots(slot_count) {
    this->name = name;
    this->valid = this->are_slots_valid();
}

void MarketSignal::set_active(const size_t& index, const bool& active) {
    SignalSlots::set_active(index, active);
    this->valid = this->are_slots_valid();
    this->mark_scheduled();
}

void MarketSignal::active_all() {
    SignalSlots::active_all();
    this->valid = this->are_slots_valid();
    this->mark_scheduled();
}

void MarketSignal::deactivate_all() {
    SignalSlots::deactivate_all();
    this->valid = this->are_slots_valid();
    this->mark_scheduled();
}

bool MarketSignal::on_parent_invalid() {
    for (size_t index = 0; index < this->get_slot_count(); ++index) {
        if (this->is_active(index)) {
            this->set_slot_output(index, quiet_nan(), false);
        }
    }

    this->refresh_global_slot_validity();
    const bool previous_validity = this->valid;
    this->valid = this->are_slots_valid();
    return previous_validity != this->valid;
}

ParametrizedMarketSignal::ParametrizedMarketSignal()
    : MarketSignal(),
      param_storage() {}

ParametrizedMarketSignal::ParametrizedMarketSignal(const string& name,
                                                   const string& instrument,
                                                   const string& exchange,
                                                   const size_t& hyperparameter_dimension,
                                                   const size_t& hyperparameter_vector_count)
    : MarketSignal(name, instrument, exchange, hyperparameter_vector_count),
      param_storage(hyperparameter_dimension, hyperparameter_vector_count) {}

size_t ParametrizedMarketSignal::get_hyperparameter_dimension() const {
    return this->param_storage.get_hyperparameter_dimension();
}

size_t ParametrizedMarketSignal::get_hyperparameter_vector_count() const {
    return this->param_storage.get_hyperparameter_vector_count();
}

const double* ParametrizedMarketSignal::get_hyperparameter_vector_data(const size_t& index) const {
    return this->param_storage.get_hyperparameter_vector_data(index);
}

double* ParametrizedMarketSignal::get_hyperparameter_vector_data(const size_t& index) {
    return this->param_storage.get_hyperparameter_vector_data(index);
}

void ParametrizedMarketSignal::set_hyperparameter_vector(const size_t& index,
                                                         const vector<double>& hyperparameter_vector) {
    this->param_storage.set_hyperparameter_vector(index, hyperparameter_vector);
}

bool ParametrizedMarketSignal::recompute() {
    auto update_slot_and_track_change = [&](const size_t& index,
                                            const double& value,
                                            const bool& valid) {
        const double previous_value = this->values[index];
        const bool previous_validity = this->valid_mask[index] != 0;
        this->set_slot_output(index, value, valid);
        return !are_equal_with_nan(previous_value, this->values[index])
            || previous_validity != (this->valid_mask[index] != 0);
    };

    bool any_vector_changed = false;

    for (size_t index = 0; index < this->get_slot_count(); ++index) {
        if (!this->is_active(index)) {
            any_vector_changed = update_slot_and_track_change(index, quiet_nan(), true) || any_vector_changed;
            continue;
        }

        double computed_value = quiet_nan();
        bool computed_valid = this->compute_active_vector(
            index,
            this->param_storage.get_hyperparameter_vector_data(index),
            computed_value
        );
        if (!computed_valid || !std::isfinite(computed_value)) {
            computed_value = quiet_nan();
            computed_valid = false;
        }

        any_vector_changed = update_slot_and_track_change(index, computed_value, computed_valid) || any_vector_changed;
    }

    const bool slot_validity_changed = this->refresh_global_slot_validity();
    const bool previous_validity = this->valid;
    this->valid = this->are_slots_valid();
    const bool node_validity_changed = previous_validity != this->valid;

    return any_vector_changed || slot_validity_changed || node_validity_changed;
}
