#include "Core/Engine/BacktestEngine.h"
#include "Core/Engine/LiveEngine.h"
#include "Core/Graph/Graph.h"
#include "Core/Node/Quote/Vwap.h"
#include "Helper/SaphirManager.h"
#include "Logger/Logger.h"

int main() {
    Logger logger("MainLogger", "/home/hugo/gte_logs");
    Graph graph(&logger);
    Vwap vwap("BTCUSDT","binance",100000);
    // Bary bary("BTCUSDT","binance");

    vwap.connect(graph);


    // BacktestEngine backtest_engine(&logger,&graph);
    LiveEngine live_engine(&logger,&graph);

    live_engine.run();


    // backtest_engine.run(
        // Timestamp(Date(2023,06,25),Time(7,00,00)),
        // Timestamp(Date(2023,06,25),Time(23,00,00))
        // );




    // LiveEngine live_engine(&logger, &graph);
    // live_engine.run();
}
