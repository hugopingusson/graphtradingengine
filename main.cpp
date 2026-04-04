#include "Core/Engine/LiveEngine.h"
#include "Core/Graph/Graph.h"
#include "Core/Node/Base/MarketNode.h"
#include "Core/Node/Signals/MathSignal.h"
#include "Core/Node/Signals/OrderBookSignal.h"
#include "Logger/Logger.h"

int main() {
    Logger logger("MainLogger", "/home/hugo/gte_logs");

    Graph graph(&logger);
    MarketOrderBook market("BTCUSDT", "binance", 10, 0.5);
    TopOfBookImbalance tob_imbalance(&market);
    Print print_imbalance(&tob_imbalance, "tob_imbalance");

    graph.add_producer(&market);
    graph.add_edge(&market, &tob_imbalance);
    graph.add_edge(&tob_imbalance, &print_imbalance);

    LiveEngine live_engine(&logger, &graph);
    live_engine.run();
}
