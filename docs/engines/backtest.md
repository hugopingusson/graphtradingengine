# Backtest Engine

File: `Core/Engine/BacktestEngine.cpp`

## Responsibilities

- Initialize graph update path.
- Build streamers from graph producers.
- Route streamers to requested backtest window.
- Merge events by timestamp and push them through graph.

## Initialization

`BacktestEngine::initialize()`:
1. `graph->resolve_update_path()`
2. `build_streamer_container()`

`build_streamer_container()` delegates registration to `BackTestStreamerContainer`.

## Streamer Registration

`BackTestStreamerContainer::register_source` supports:
- `MarketOrderBook` -> `DatabaseWMBPBacktestStreamer`
- `HeartBeat` -> `HeartBeatBackTestStreamer`

## Runtime Loop

`BacktestEngine::run(start, end)`:
1. Route streamers with `route_and_set_streamers(start, end)`.
2. Fill min-heap with current item from each streamer.
3. Repeatedly pop smallest timestamp:
   - `streamer->process_current(graph)`
   - `streamer->advance()`
   - reinsert streamer if it still has data

Ordering key is `HeapItem.row`, currently bound to capture timestamp.

## Data Input Contract

`DatabaseWMBPBacktestStreamer` reads binary rows of:
- `BacktestWideMarketByPriceRow`

Current behavior:
- It emits a `SnapshotEvent` from each row (snapshot payload).
- The embedded row `order` field is not emitted as an `OrderEvent` in current implementation.

## Heartbeat Streamer

`HeartBeatBackTestStreamer`:
- initializes at `start.unixtime()`
- increments by `1e9 * frequency`
- stops at `end.unixtime()`

