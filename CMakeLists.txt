cmake_minimum_required(VERSION 3.30)
project(GraphTradingEngine)
set(CMAKE_CXX_STANDARD 20)


find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem thread)


include_directories(${Boost_INCLUDE_DIRS})



add_executable(GraphTradingEngine main.cpp
        Data/DataReader/ParquetDataReader.h
        Data/DataReader/ParquetDataReader.cpp
        Helper/FutureHelper.cpp
        Helper/FutureHelper.h
        Helper/TimeHelper.cpp
        Helper/TimeHelper.h
        Core/Node/Base/Node.cpp
        Core/Node/Base/Node.h
        Logger/Logger.cpp
        Logger/Logger.h
        Core/Graph/Event.cpp
        Core/Graph/Event.h
        Core/Node/Base/MarketNode.cpp
        Core/Node/Base/MarketNode.h
        Core/Graph/Graph.cpp
        Core/Graph/Graph.h
        Core/Node/Signals/OrderBookSignal.cpp
        Core/Node/Signals/OrderBookSignal.h
        Core/Node/Signals/MathSignal.cpp
        Core/Node/Signals/MathSignal.h
        Core/Streamer/Streamer.cpp
        Core/Streamer/Streamer.h
        Core/Engine/BacktestEngine.cpp
        Core/Engine/BacktestEngine.h
        Helper/ToolBox.cpp
        Helper/ToolBox.h
        Core/Node/Base/HeartBeat.cpp
        Core/Node/Base/HeartBeat.h
        Core/Graph/EventHandler.h
        Data/DataStructure/DataStructure.h
        Helper/DataBaseHelper.cpp
        Helper/DataBaseHelper.h
)


target_link_libraries(GraphTradingEngine boost_date_time arrow parquet boost_filesystem fmt)

target_link_libraries(GraphTradingEngine
        ${Boost_LOG_LIBRARY}    # Boost.Log library
        ${Boost_THREAD_LIBRARY}  # Boost.Thread library
        ${Boost_SYSTEM_LIBRARY}  # Boost.System library
        ${Boost_FILESYSTEM_LIBRARY} # Boost.Filesystem library
)