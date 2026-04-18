# Extension Playbook

## Add A New Signal Node

1. Create class under `Core/Node/Signals/`.
2. Inherit from `SingleInputConsumer`.
3. Keep `compute()` focused on:
   - set `value`
   - set `valid=false` only on computation failure/invalid math
4. Build graph edges from parent producer/consumer to new signal.

Recommended contract:
- Parent validity is checked in `SingleInputConsumer::update()`.
- `compute()` should not check graph dirty state.

## Add A New Live Exchange Streamer

1. Create streamer class in `Core/Streamer/`.
2. Pick base:
   - `LiveSnapshotOrderBookStreamer` for snapshot-only feeds.
   - `LiveUpdateDeltaOrderBookStreamer` for snapshot+delta update feeds.
   - `LiveOrderDeltaOrderBookStreamer` for snapshot+order-delta feeds.
3. Implement `run_loop()`:
   - connect websocket
   - decode payload
   - emit events via `emit_*_event(...)`
4. Register exchange in `LiveEngine::register_market_orderbook_source`.

Guidelines:
- Normalize timestamps to nanoseconds.
- Set `source_id_trigger` to associated producer node id.
- Emit snapshot first for delta feeds (`mark_bootstrapped()`).

## Add A New Producer Type

1. Create node class inheriting `Producer`.
2. Implement `on_event(Event*)`.
3. Add handling in:
   - `BackTestStreamerContainer::register_source` if backtest support needed.
   - `LiveEngine::register_source` if live support needed.

## Change Backtest Event Mapping

Current `DatabaseWMBPBacktestStreamer` emits only `SnapshotEvent`.

To support additional event semantics:
1. Extend row format and converter write path.
2. In `process_current`, emit `OrderEvent` or `UpdateEvent` based on row content.
3. Keep ordering key consistent with chosen timestamp semantics.

## Performance-Critical Areas

- `MarketOrderBook` ladder structure is `std::map` (logN operations, pointer-heavy).
- Live consumer loop heap/rescan logic dominates when streamer count grows.
- JSON websocket parsing dominates CPU for crypto exchanges.

