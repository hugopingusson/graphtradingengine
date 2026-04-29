//
// Created by hugo on 28/04/26.
//

#ifndef SIGNALBASE_H
#define SIGNALBASE_H

#include "../Base/Node.h"

class Signal : public Consumer {
public:
    ~Signal() override = default;
    Signal();
    Signal(const string& name,
           const size_t& slot_count);

    [[nodiscard]] size_t get_slot_count() const;

    [[nodiscard]] bool is_active(const size_t& index) const;
    void set_active(const size_t& index, const bool& active);

    [[nodiscard]] const vector<double>& get_values() const;
    [[nodiscard]] double get_value(const size_t& index) const;
    [[nodiscard]] const vector<uint8_t>& get_active_mask() const;
    [[nodiscard]] const vector<uint8_t>& get_valid_mask() const;
    [[nodiscard]] bool is_slot_valid(const size_t& index) const;

protected:
    void set_slot_output(const size_t& index,
                         const double& value,
                         const bool& valid);
    bool refresh_global_validity();

    vector<double> values;
    vector<uint8_t> active_mask;
    vector<uint8_t> valid_mask;
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

class ParametrizedSignal : public Signal {
public:
    ~ParametrizedSignal() override = default;
    ParametrizedSignal();
    ParametrizedSignal(const string& name,
                       const size_t& hyperparameter_dimension,
                       const size_t& hyperparameter_vector_count);

    [[nodiscard]] size_t get_hyperparameter_dimension() const;
    [[nodiscard]] size_t get_hyperparameter_vector_count() const;
    [[nodiscard]] const double* get_hyperparameter_vector_data(const size_t& index) const;
    [[nodiscard]] double* get_hyperparameter_vector_data(const size_t& index);
    void set_hyperparameter_vector(const size_t& index,
                                   const vector<double>& hyperparameter_vector);

protected:
    ParamStorage param_storage;
};

class MarketSignal : public Signal {
public:
    ~MarketSignal() override = default;
    MarketSignal();
    MarketSignal(const string& name,
                 const string& instrument,
                 const string& exchange,
                 const size_t& slot_count);

    Market* connect(Graph& graph);
    bool update() override;

    [[nodiscard]] const string& get_instrument() const;
    [[nodiscard]] const string& get_exchange() const;
    [[nodiscard]] Market* get_market_parent() const;

protected:
    virtual bool compute_active_slot(const size_t& index,
                                     double& out_value) = 0;

    Market* market_parent;
    string instrument;
    string exchange;
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
    bool compute_active_slot(const size_t& index,
                             double& out_value) override;
    virtual bool compute_active_vector(const size_t& index,
                                       const double* hyperparameter_vector,
                                       double& out_value) = 0;

    ParamStorage param_storage;
};

#endif //SIGNALBASE_H
