//
// Created by hugo on 28/04/26.
//

#include "Base.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "../../Graph/Graph.h"

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

Signal::Signal()
    : Consumer(),
      values(),
      active_mask(),
      valid_mask() {}

Signal::Signal(const string& name,
               const size_t& slot_count)
    : Node(),
      Consumer(),
      values(),
      active_mask(),
      valid_mask() {
    this->name = name;
    this->values.assign(slot_count, quiet_nan());
    this->active_mask.assign(slot_count, 1);
    this->valid_mask.assign(slot_count, 0);
    this->valid = (slot_count == 0);
}

size_t Signal::get_slot_count() const {
    return this->values.size();
}

bool Signal::is_active(const size_t& index) const {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("Signal::is_active index out of range");
    }
    return this->active_mask[index] != 0;
}

void Signal::set_active(const size_t& index, const bool& active) {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("Signal::set_active index out of range");
    }

    this->active_mask[index] = active ? 1 : 0;
    if (!active) {
        this->values[index] = quiet_nan();
        this->valid_mask[index] = 1;
    } else {
        this->values[index] = quiet_nan();
        this->valid_mask[index] = 0;
    }

    this->refresh_global_validity();
    this->mark_scheduled();
}

const vector<double>& Signal::get_values() const {
    return this->values;
}

double Signal::get_value(const size_t& index) const {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("Signal::get_value index out of range");
    }
    return this->values[index];
}

const vector<uint8_t>& Signal::get_active_mask() const {
    return this->active_mask;
}

const vector<uint8_t>& Signal::get_valid_mask() const {
    return this->valid_mask;
}

bool Signal::is_slot_valid(const size_t& index) const {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("Signal::is_slot_valid index out of range");
    }
    return this->valid_mask[index] != 0;
}

void Signal::set_slot_output(const size_t& index,
                             const double& value,
                             const bool& valid) {
    if (index >= this->get_slot_count()) {
        throw std::out_of_range("Signal::set_slot_output index out of range");
    }
    this->values[index] = value;
    this->valid_mask[index] = valid ? 1 : 0;
}

bool Signal::refresh_global_validity() {
    const bool previous_validity = this->valid;
    bool new_validity = true;
    for (const uint8_t vector_validity : this->valid_mask) {
        new_validity = new_validity && (vector_validity != 0);
    }
    this->valid = new_validity;
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
    : Signal(),
      param_storage() {}

ParametrizedSignal::ParametrizedSignal(const string& name,
                                       const size_t& hyperparameter_dimension,
                                       const size_t& hyperparameter_vector_count)
    : Signal(name, hyperparameter_vector_count),
      param_storage(hyperparameter_dimension, hyperparameter_vector_count) {}

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
    : Signal(),
      market_parent(nullptr),
      instrument(),
      exchange() {}

MarketSignal::MarketSignal(const string& name,
                           const string& instrument,
                           const string& exchange,
                           const size_t& slot_count)
    : Node(),
      Signal(name, slot_count),
      market_parent(nullptr),
      instrument(instrument),
      exchange(exchange) {}

Market* MarketSignal::connect(Graph& graph) {
    if (this->instrument.empty()) {
        throw std::runtime_error("MarketSignal::connect cannot connect without instrument");
    }

    Market* selected_market = graph.ensure_market(this->instrument, this->exchange);
    this->market_parent = selected_market;
    this->instrument = selected_market->get_instrument();
    this->exchange = selected_market->get_exchange();

    graph.add_edge(selected_market, this);
    this->mark_scheduled();
    return selected_market;
}

bool MarketSignal::update() {
    if (!this->market_parent) {
        throw std::runtime_error("MarketSignal::update called without connected market parent");
    }

    const bool parent_valid = this->market_parent->is_valid();
    bool any_vector_changed = false;

    for (size_t index = 0; index < this->get_slot_count(); ++index) {
        if (!this->is_active(index)) {
            const double previous_value = this->values[index];
            const bool previous_validity = this->valid_mask[index] != 0;
            this->set_slot_output(index, quiet_nan(), true);
            if (!are_equal_with_nan(previous_value, this->values[index])
                || previous_validity != (this->valid_mask[index] != 0)) {
                any_vector_changed = true;
            }
            continue;
        }

        if (!parent_valid) {
            const double previous_value = this->values[index];
            const bool previous_validity = this->valid_mask[index] != 0;
            this->set_slot_output(index, quiet_nan(), false);
            if (!are_equal_with_nan(previous_value, this->values[index])
                || previous_validity != (this->valid_mask[index] != 0)) {
                any_vector_changed = true;
            }
            continue;
        }

        double computed_value = quiet_nan();
        bool computed_valid = this->compute_active_slot(index, computed_value);

        if (!computed_valid || !std::isfinite(computed_value)) {
            computed_value = quiet_nan();
            computed_valid = false;
        }

        const double previous_value = this->values[index];
        const bool previous_validity = this->valid_mask[index] != 0;
        this->set_slot_output(index, computed_value, computed_valid);
        if (!are_equal_with_nan(previous_value, this->values[index])
            || previous_validity != (this->valid_mask[index] != 0)) {
            any_vector_changed = true;
        }
    }

    const bool global_validity_changed = this->refresh_global_validity();
    return any_vector_changed || global_validity_changed;
}

const string& MarketSignal::get_instrument() const {
    return this->instrument;
}

const string& MarketSignal::get_exchange() const {
    return this->exchange;
}

Market* MarketSignal::get_market_parent() const {
    return this->market_parent;
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

bool ParametrizedMarketSignal::compute_active_slot(const size_t& index,
                                                   double& out_value) {
    return this->compute_active_vector(
        index,
        this->param_storage.get_hyperparameter_vector_data(index),
        out_value
    );
}
