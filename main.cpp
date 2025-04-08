#include <iostream>

#include "Core/Engine/BacktestEngine.h"
#include "Core/Node/Base//Node.h"
#include "DataReader/DataReader.h"
#include "Helper/FutureHelper.h"
#include "Logger/Logger.h"
#include "Core/Node/Base/Node.h"
#include "Core/Node/Base/MarketNode.h"
#include "Core/Graph/Graph.h"
#include "Core/Node/Signals/MathSignal.h"
#include "Core/Node/Signals/OrderBookSignal.h"
#include "Core/Streamer/Streamer.h"


// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {

    // DataReader data_reader = DataReader();
    // auto test = data_reader.get_cme_market_data_table("EURUSD","2024-04-29");
    Logger logger = Logger("MainLogger","/home/hugo/gte_logs");
    Graph graph=Graph(&logger);
    MarketOrderBook market=MarketOrderBook("EURUSD","cme",5,1e-5);
    MarketOrderBook market_jpy=MarketOrderBook("USDJPY","cme",5,1e-5);
    HeartBeat heart_beat = HeartBeat(0.1);

    Mid mid = Mid(&market);
    Bary bary=Bary(&market);
    Bary bary_jpy=Bary(&market_jpy);
    Skew skew=Skew(&bary,&mid);
    Skew skew_jpy=Skew(&bary_jpy,&skew);

    graph.add_source(&market);
    graph.add_source(&market_jpy);


    graph.add_edge(&heart_beat,&market);
    graph.add_edge(&market,&mid);
    graph.add_edge(&market,&bary);
    graph.add_edge(&market_jpy,&bary_jpy);

    graph.add_edge(&bary,&skew);
    graph.add_edge(&mid,&skew);

    graph.add_edge(&bary_jpy,&skew_jpy);
    graph.add_edge(&skew,&skew_jpy);

    BacktestEngine  backtest_engine = BacktestEngine(&logger,&graph);
    backtest_engine.initialize();

    cout << "Done" << endl;
    return 1;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.