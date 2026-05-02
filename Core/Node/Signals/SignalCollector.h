//
// Created by hugo on 30/04/26.
//

#ifndef SIGNALCOLLECTOR_H
#define SIGNALCOLLECTOR_H

#include "../Base/Node.h"
#include "Base.h"

class SignalCollector : public Consumer {
public:
    SignalCollector();
    explicit SignalCollector(const string& name);
    ~SignalCollector() override = default;

    void add_signal(Graph& graph, Consumer& signal_owner, SignalSlots& signal_slots);

    bool update() override;

    [[nodiscard]] size_t get_signal_count() const;
    [[nodiscard]] size_t get_feature_count() const;
    [[nodiscard]] const vector<double>& get_features() const;
    [[nodiscard]] const vector<uint8_t>& get_feature_valid_mask() const;
    [[nodiscard]] bool is_all_valid() const;

private:
    struct SignalBinding {
        Consumer* owner;
        SignalSlots* slots;
    };

    vector<SignalBinding> signal_bindings;
    size_t total_slot_count;
    vector<double> features;
    vector<uint8_t> feature_valid_mask;
    bool all_valid;
};

#endif //SIGNALCOLLECTOR_H
