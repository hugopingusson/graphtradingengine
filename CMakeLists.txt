cmake_minimum_required(VERSION 3.22)
project(GraphTradingEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pour le warning CMP0167 (FindBoost), on force l'ancien comportement
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# Dépendances
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem thread date_time log)
find_package(Threads REQUIRED)

# Arrow (si tu en as besoin)
find_package(Arrow QUIET) # passe à REQUIRED si indispensable

# ---- SOURCES : supprime tout ce qui touche à Parquet ----
add_executable(GraphTradingEngine
        main.cpp
        Helper/FutureHelper.cpp
        Helper/TimeHelper.cpp
        Helper/ToolBox.cpp
        Helper/DataBaseHelper.cpp
        Logger/Logger.cpp
        Core/Graph/Event.cpp
        Core/Graph/Graph.cpp
        Core/Node/Base/Node.cpp
        Core/Node/Base/MarketNode.cpp
        Core/Node/Base/HeartBeat.cpp
        Core/Node/Signals/OrderBookSignal.cpp
        Core/Node/Signals/MathSignal.cpp
        Core/Streamer/Streamer.cpp
        Core/Engine/BacktestEngine.cpp

        # ⚠️ Ajoute ici les .cpp réellement utilisés par main :
        # Core/Node/OrderBook/MarketOrderBook.cpp
        # Core/Node/Trades/MarketTrade.cpp
        # Core/Node/Signals/Mid.cpp
        # Core/Node/Signals/Bary.cpp
        # Core/Node/Signals/Skew.cpp
)

target_include_directories(GraphTradingEngine PRIVATE ${Boost_INCLUDE_DIRS})

# fmt en header-only (évite le lien binaire et les erreurs fmt::v6)
target_compile_definitions(GraphTradingEngine PRIVATE FMT_HEADER_ONLY)

# Lien (sans Parquet)
target_link_libraries(GraphTradingEngine PRIVATE
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_DATE_TIME_LIBRARY}
        ${Boost_LOG_LIBRARY}
        Threads::Threads
)

# Lier Arrow seulement si trouvé
if (TARGET Arrow::arrow)
    target_link_libraries(GraphTradingEngine PRIVATE Arrow::arrow)
elseif (DEFINED ARROW_SHARED_LIB)
    target_link_libraries(GraphTradingEngine PRIVATE ${ARROW_SHARED_LIB})
endif()

# Warnings
if (MSVC)
    target_compile_options(GraphTradingEngine PRIVATE /W4)
else()
    target_compile_options(GraphTradingEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()
