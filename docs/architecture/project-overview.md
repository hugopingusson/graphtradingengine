# Architecture Overview

## Project Goal

GraphTradingEngine executes trading logic as a directed graph of nodes.
- Producers receive external events (`MarketOrderBook`, `HeartBeat`).
- Consumers recompute derived signals (`Mid`, `Bary`, `Vwap`, `TopOfBookImbalance`, `Print`).
- The graph propagates updates only along registered dependencies.

Two runtime modes exist:
- Backtest: replay historical binary rows.
- Live: consume real-time market feeds from websocket streamers.

## High-Level Runtime Flow

1. Build a `Graph` and attach producer nodes.
2. Attach consumer nodes with `graph.add_edge(parent, child)`.
3. Engine resolves graph update order (`resolve_update_path()`).
4. Streamers emit `Event` objects with `source_id_trigger`.
5. Engine routes each event to the matching producer.
6. Producer updates internal state.
7. `graph.update(source_id)` recomputes downstream dirty nodes.

## Main Entry Point

Current `main.cpp` runs a live example:
- `MarketOrderBook("BTCUSDT", "binance", ...)`
- `TopOfBookImbalance`
- `Print`
- `LiveEngine.run()`

## Core Design Choices

- Event polymorphism + double dispatch: `Event::dispatchTo(MarketOrderBook&)`.
- Graph update is single-threaded and deterministic per source path.
- Live ingestion is multi-threaded on input side (one thread per streamer), single-threaded on graph execution side.
- Binary backtest rows are fixed-layout packed structs for fast sequential IO.

