#include <iostream>

#include "Core/Node/Node.h"
#include "DataReader/DataReader.h"
#include "Helper/FutureHelper.h"
#include "Logger/Logger.h"
#include "Core/Node/Node.h"
#include "Core/Node/Market.h"
#include "Core/Graph/Graph.h"
#include "Core/Node/Signals/MathNode.h"
#include "Core/Node/Signals/OrberBook.h"


// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main() {

    // DataReader data_reader = DataReader();
    // auto test = data_reader.get_cme_market_data_table("EURUSD","2024-04-29");

    Graph graph=Graph();
    Market market=Market("EURUSD","cme",5,1e-5);
    Market market_jpy=Market("USDJPY","cme",5,1e-5);
    Mid mid = Mid(&market);
    Bary bary=Bary(&market);

    Bary bary_jpy=Bary(&market_jpy);

    Skew skew=Skew(&bary,&mid);

    Skew skew_jpy=Skew(&bary_jpy,&skew);

    graph.add_source(&market);
    graph.add_source(&market_jpy);

    graph.add_edge(&market,&mid);
    graph.add_edge(&market,&bary);
    graph.add_edge(&market_jpy,&bary_jpy);

    graph.add_edge(&bary,&skew);
    graph.add_edge(&mid,&skew);

    graph.add_edge(&bary_jpy,&skew_jpy);
    graph.add_edge(&skew,&skew_jpy);

    graph.resolve_update_path();

    // Logger logger = Logger();
    // logger.log_error("Ceci est une error");
    // logger.log_info("Ceci est une info");
    // auto file_logger = spdlog::basic_logger_mt("file_logger", "logs.txt");
    return 1;
}

// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.