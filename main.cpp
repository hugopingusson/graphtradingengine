#include "Core/Engine/LiveEngine.h"
#include "Core/Graph/Graph.h"
#include "Core/Node/Base/MarketNode.h"
#include "Core/Node/Signals/MathSignal.h"
#include "Core/Node/Signals/OrderBookSignal.h"
#include "Logger/Logger.h"

int main() {
    Logger logger("MainLogger", "/home/hugo/gte_logs");

    Graph graph(&logger);
    MarketOrderBook market("XBTUSD", "bitmex", 10, 0.5);
    Vwap vwap(&market, 200.0);
    Print print_mid(&vwap, "mid");

    graph.add_producer(&market);
    graph.add_edge(&market, &vwap);
    graph.add_edge(&vwap, &print_mid);

    LiveEngine live_engine(&logger, &graph);
    live_engine.run();
}
