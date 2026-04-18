# Build And Run

## Toolchain

Defined in root `CMakeLists.txt`:
- C++20
- Boost (`system`, `filesystem`, `thread`, `date_time`, `log`)
- OpenSSL
- Arrow + Parquet
- spdlog
- fmt (header-only mode)

## Build

```bash
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release -j
```

Executable:
- `cmake-build-release/GraphTradingEngine`

## Current Main Example

`main.cpp` currently creates:
- `MarketOrderBook(BTCUSDT, binance)`
- `TopOfBookImbalance`
- `Print`
- `LiveEngine.run()`

So binary runs in live mode and blocks while consuming market data.

## Logging

`Logger` writes async file logs under a directory passed at construction, for example:
- `Logger("MainLogger", "/home/hugo/gte_logs")`

If logger path does not exist, constructor throws.

## Time Utilities

`Helper/TimeHelper` provides:
- `Date`, `Time`, `Timestamp`
- conversions to unix seconds/nanoseconds
- parser for `"yyyy-mm-dd HH:MM:SS"`
- `Timestamp::unix_to_string(...)` -> `yyyy-mm-dd HH:MM:SS.NNNNNNNNN`

