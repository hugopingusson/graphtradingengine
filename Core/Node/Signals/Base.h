//
// Created by hugo on 28/04/26.
//

#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include "../Base/Node.h"

class SignalSlots {
public:
    SignalSlots();
    explicit SignalSlots(const size_t& slot_count);

    [[nodiscard]] size_t get_slot_count() const;

    [[nodiscard]] bool is_active(const size_t& index) const;
    void set_active(const size_t& index, const bool& active);
    void active_all();
    void deactivate_all();

    [[nodiscard]] const vector<double>& get_values() const;
    [[nodiscard]] double get_value(const size_t& index) const;
    [[nodiscard]] const vector<uint8_t>& get_active_mask() const;
    [[nodiscard]] const vector<uint8_t>& get_valid_mask() const;
    [[nodiscard]] bool is_slot_valid(const size_t& index) const;
    [[nodiscard]] bool are_slots_valid() const;

protected:
    void set_slot_output(const size_t& index,
                         const double& value,
                         const bool& valid);
    bool refresh_global_slot_validity();

    vector<double> values;
    vector<uint8_t> active_mask;
    vector<uint8_t> valid_mask;
    bool slots_valid;
};

class ParamStorage {
public:
    ParamStorage();
    ParamStorage(const size_t& hyperparameter_dimension,
                 const size_t& hyperparameter_vector_count);

    [[nodiscard]] size_t get_hyperparameter_dimension() const;
    [[nodiscard]] size_t get_hyperparameter_vector_count() const;

    [[nodiscard]] const double* get_hyperparameter_vector_data(const size_t& index) const;
    [[nodiscard]] double* get_hyperparameter_vector_data(const size_t& index);
    void set_hyperparameter_vector(const size_t& index,
                                   const vector<double>& hyperparameter_vector);

protected:
    size_t hyperparameter_dimension;
    size_t hyperparameter_vector_count;
    vector<double> hyperparameter_space;
};

class ParametrizedSignal : public SignalSlots {
public:
    ~ParametrizedSignal() = default;
    ParametrizedSignal();
    ParametrizedSignal(const string& name,
                       const size_t& hyperparameter_dimension,
                       const size_t& hyperparameter_vector_count);

    [[nodiscard]] const string& get_name() const;

    [[nodiscard]] size_t get_hyperparameter_dimension() const;
    [[nodiscard]] size_t get_hyperparameter_vector_count() const;
    [[nodiscard]] const double* get_hyperparameter_vector_data(const size_t& index) const;
    [[nodiscard]] double* get_hyperparameter_vector_data(const size_t& index);
    void set_hyperparameter_vector(const size_t& index,
                                   const vector<double>& hyperparameter_vector);

protected:
    string name;
    ParamStorage param_storage;
};

class MarketSignal : public MarketConsumer, public SignalSlots {
public:
    ~MarketSignal() override = default;
    MarketSignal();
    MarketSignal(const string& name,
                 const string& instrument,
                 const string& exchange,
                 const size_t& slot_count);

    void set_active(const size_t& index, const bool& active);
    void active_all();
    void deactivate_all();

protected:
    bool on_parent_invalid() override;
    bool recompute() override = 0;
};




class ParametrizedMarketSignal : public MarketSignal {
public:
    ~ParametrizedMarketSignal() override = default;
    ParametrizedMarketSignal();
    ParametrizedMarketSignal(const string& name,
                             const string& instrument,
                             const string& exchange,
                             const size_t& hyperparameter_dimension,
                             const size_t& hyperparameter_vector_count);

    [[nodiscard]] size_t get_hyperparameter_dimension() const;
    [[nodiscard]] size_t get_hyperparameter_vector_count() const;
    [[nodiscard]] const double* get_hyperparameter_vector_data(const size_t& index) const;
    [[nodiscard]] double* get_hyperparameter_vector_data(const size_t& index);
    void set_hyperparameter_vector(const size_t& index,
                                   const vector<double>& hyperparameter_vector);

protected:
    bool recompute() override;
    virtual bool compute_active_vector(const size_t& index,
                                       const double* hyperparameter_vector,
                                       double& out_value) = 0;

    ParamStorage param_storage;
};

#endif //SIGNALBASE_H
