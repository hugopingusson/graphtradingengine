cmake_minimum_required(VERSION 3.22)
project(GraphTradingEngine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Keep legacy FindBoost behavior for CMP0167
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# Dependencies
find_package(Boost 1.71 REQUIRED COMPONENTS system filesystem thread date_time log)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Arrow / Parquet: force a single consistent distro stack (system 19.0.1 with snappy).
set(Arrow_DIR "/usr/lib/x86_64-linux-gnu/cmake/Arrow" CACHE PATH "Arrow config path" FORCE)
set(Parquet_DIR "/usr/lib/x86_64-linux-gnu/cmake/Parquet" CACHE PATH "Parquet config path" FORCE)
find_package(Arrow REQUIRED CONFIG)
find_package(Parquet REQUIRED CONFIG)

# ---- SOURCES: include only files needed by the current executable ----
add_executable(GraphTradingEngine
        main.cpp
        Helper/FutureHelper.cpp
        Helper/TimeHelper.cpp
        Helper/DataBaseHelper.cpp
        Helper/SaphirManager.cpp
        Logger/Logger.cpp
        Core/Graph/Event.cpp
        Core/Graph/Graph.cpp
        Core/Node/Base/Node.cpp
        Core/Node/Base/MarketNode.cpp
        Core/Node/Quote/Quote.cpp
        Core/Node/Quote/Vwap.cpp
        Core/Node/Signals/Base.cpp
        Core/Node/Signals/OrderBookSignal.cpp
        Core/Streamer/Streamer.cpp
        Core/Streamer/LiveStreamer.cpp
        Core/Streamer/BitmexLiveOrderBookStreamer.cpp
        Core/Streamer/BinanceLiveOrderBookStreamer.cpp
        Core/Streamer/DeribitLiveOrderBookStreamer.cpp
        Core/Streamer/OkxLiveOrderBookStreamer.cpp
        Core/Engine/BacktestEngine.cpp
        Core/Engine/LiveEngine.cpp
        Helper/Configuration.h
        Core/Node/Signals/Base.h
        Core/Node/Quote/Quote.h
         Core/Node/Signals/OFI.cpp
         Core/Node/Signals/OFI.h
         Component/Aggregation/PointWindow.cpp
         Component/Aggregation/TimeWindow.cpp

         # Add here the .cpp files effectively used by main:
        # Core/Node/OrderBook/MarketOrderBook.cpp
        # Core/Node/Trades/MarketTrade.cpp
        # Core/Node/Signals/Mid.cpp
        # Core/Node/Signals/Bary.cpp
        # Core/Node/Signals/Skew.cpp
)

target_include_directories(GraphTradingEngine PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})

# Use fmt in header-only mode to avoid binary-link issues and fmt::v6 mismatches
target_compile_definitions(GraphTradingEngine PRIVATE FMT_HEADER_ONLY)

# Link libraries
target_link_libraries(GraphTradingEngine PRIVATE
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_THREAD_LIBRARY}
        ${Boost_DATE_TIME_LIBRARY}
        ${Boost_LOG_LIBRARY}
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

# Link Arrow/Parquet only when targets are available
if (TARGET Arrow::arrow)
    target_link_libraries(GraphTradingEngine PRIVATE Arrow::arrow)
elseif (DEFINED ARROW_SHARED_LIB)
    target_link_libraries(GraphTradingEngine PRIVATE ${ARROW_SHARED_LIB})
endif()

if (TARGET Parquet::parquet)
    target_link_libraries(GraphTradingEngine PRIVATE Parquet::parquet)
elseif (DEFINED PARQUET_SHARED_LIB)
    target_link_libraries(GraphTradingEngine PRIVATE ${PARQUET_SHARED_LIB})
endif()

# Warnings
if (MSVC)
    target_compile_options(GraphTradingEngine PRIVATE /W4)
else()
    target_compile_options(GraphTradingEngine PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Standalone full-history converter executable (Databento + Cryptolake)
add_executable(MarketDataConverter
        tools/ConvertAllMarketData.cpp
        Data/DataReader/CMEParquetToBin.cpp
        Data/DataReader/CryptolakeParquetToBin.cpp
        Helper/FutureHelper.cpp
)

target_include_directories(MarketDataConverter PRIVATE ${Boost_INCLUDE_DIRS})
target_compile_definitions(MarketDataConverter PRIVATE FMT_HEADER_ONLY=1)

target_link_libraries(MarketDataConverter PRIVATE
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_DATE_TIME_LIBRARY}
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)

if (TARGET Arrow::arrow)
    target_link_libraries(MarketDataConverter PRIVATE Arrow::arrow)
elseif (DEFINED ARROW_SHARED_LIB)
    target_link_libraries(MarketDataConverter PRIVATE ${ARROW_SHARED_LIB})
endif()

if (TARGET Parquet::parquet)
    target_link_libraries(MarketDataConverter PRIVATE Parquet::parquet)
elseif (DEFINED PARQUET_SHARED_LIB)
    target_link_libraries(MarketDataConverter PRIVATE ${PARQUET_SHARED_LIB})
endif()

if (MSVC)
    target_compile_options(MarketDataConverter PRIVATE /W4)
else()
    target_compile_options(MarketDataConverter PRIVATE -Wall -Wextra -Wpedantic)
endif()
