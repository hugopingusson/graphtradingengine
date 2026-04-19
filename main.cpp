#include "Core/Engine/BacktestEngine.h"
#include "Core/Engine/LiveEngine.h"
#include "Core/Graph/Graph.h"
#include "Core/Node/Base/MarketNode.h"
#include "Core/Node/Signals/MathSignal.h"
#include "Core/Node/Signals/OrderBookSignal.h"
#include "Helper/SaphirManager.h"
#include "Logger/Logger.h"

int main() {
    Logger logger("MainLogger", "/home/hugo/gte_logs");
    const SaphirManager saphir;
    const int market_depth = static_cast<int>(saphir.get_market_depth());

    Graph graph(&logger);
    Market market("6EM4", "cme", market_depth, 0.5);
    TopOfBookImbalance tob_imbalance(&market);
    Print print_imbalance(&tob_imbalance, "tob_imbalance");

    graph.add_producer(&market);
    graph.add_edge(&market, &tob_imbalance);
    graph.add_edge(&tob_imbalance, &print_imbalance);

    BacktestEngine backtest_engine(&logger,&graph);

    backtest_engine.run(
        Timestamp(Date(2024,04,28),Time(12,00,00)),
        Timestamp(Date(2024,04,28),Time(13,00,00))
        );




    // LiveEngine live_engine(&logger, &graph);
    // live_engine.run();
}
