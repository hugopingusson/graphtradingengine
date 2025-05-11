#include <iostream>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include "Core/Engine/BacktestEngine.h"
#include "Core/Node/Base//Node.h"
#include "Helper/FutureHelper.h"
#include "Logger/Logger.h"
#include "Core/Node/Base/Node.h"
#include "Core/Node/Base/MarketNode.h"
#include "Core/Graph/Graph.h"
#include "Core/Node/Signals/MathSignal.h"
#include "Core/Node/Signals/OrderBookSignal.h"
#include "Core/Streamer/Streamer.h"
#include "Data/DataStructure/DataStructure.h"

// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
// int main() {
//     Logger logger = Logger("MainLogger","/home/hugo/gte_logs");
//     Graph graph=Graph(&logger);
//     MarketOrderBook market=MarketOrderBook("EURUSD","cme",5,1e-5);
//     MarketTrade market_trade = MarketTrade("EURUSD","cme");
//
//     Mid mid = Mid(&market);
//     Bary bary=Bary(&market);
//     Skew skew=Skew(&bary,&mid);
//
//     graph.add_source(&market_trade);
//     graph.add_source(&market);
//
//
//     graph.add_edge(&market,&mid);
//     graph.add_edge(&market,&bary);
//
//     graph.add_edge(&bary,&skew);
//     graph.add_edge(&mid,&skew);
//
//
//     BacktestEngine  backtest_engine = BacktestEngine(&logger,&graph);
//     backtest_engine.initialize();
//     backtest_engine.run("2024-02-01");
//
//
//
//
//
// }


int main() {
    // std::ifstream file("/media/hugo/T7/market_data/databento/mbp10_bin/6EM4/2024-05-01.bin", std::ios::binary);
    // MarketByPriceSnapshot row;
    // file.read(reinterpret_cast<char*>(&row), sizeof(MarketByPriceSnapshot));
    // while (file.read(reinterpret_cast<char*>(&row), sizeof(MarketByPriceSnapshot))) {;  // strip padding
    //     std::cout << "ts: " << row.market_timestamp.capture_server_in_timestamp
    //     << ", ask: " << row.order_book_snapshot_data.ask_price[0]
    //     << ", action: " << row.action_data.base_quantity
    //     << ", bid: " << row.order_book_snapshot_data.bid_price[0] << endl;
    // }
    // std::cout << "done" << std::endl;


    Logger logger = Logger("MainLogger","/home/hugo/gte_logs");
    Graph graph=Graph(&logger);
    MarketOrderBook market=MarketOrderBook("EURUSD","cme",5,1e-5);
    // MarketTrade market_trade = MarketTrade("EURUSD","cme");
    // Mid mid = Mid(&market);
    Bary bary=Bary(&market);
    // Skew skew=Skew(&bary,&mid);
    // graph.add_source(&market_trade);
    graph.add_source(&market);
    // graph.add_edge(&market,&mid);
    graph.add_edge(&market,&bary);
    // graph.add_edge(&bary,&skew);
    // graph.add_edge(&mid,&skew);
    BacktestEngine backtest_engine = BacktestEngine(&logger,&graph);
    backtest_engine.initialize();
    backtest_engine.run("2024-05-01");

    return 1;
}// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.