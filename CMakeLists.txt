cmake_minimum_required(VERSION 3.30)
project(GraphTradingEngine)
set(CMAKE_CXX_STANDARD 20)


find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem thread)


include_directories(${Boost_INCLUDE_DIRS})



add_executable(GraphTradingEngine main.cpp
        DataReader/DataReader.h
        DataReader/DataReader.cpp
        Helper/FutureHelper.cpp
        Helper/FutureHelper.h
        Helper/DateHelper.cpp
        Helper/DateHelper.h
        Core/Node/Node.cpp
        Core/Node/Node.h
        Logger/Logger.cpp
        Logger/Logger.h
        Core/Graph/MarketEvent.cpp
        Core/Graph/MarketEvent.h
        Core/Node/Market.cpp
        Core/Node/Market.h
        Core/Graph/Graph.cpp
        Core/Graph/Graph.h
)


target_link_libraries(GraphTradingEngine boost_date_time arrow parquet boost_filesystem fmt)

target_link_libraries(GraphTradingEngine
        ${Boost_LOG_LIBRARY}    # Boost.Log library
        ${Boost_THREAD_LIBRARY}  # Boost.Thread library
        ${Boost_SYSTEM_LIBRARY}  # Boost.System library
        ${Boost_FILESYSTEM_LIBRARY} # Boost.Filesystem library
)