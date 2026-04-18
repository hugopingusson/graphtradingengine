# Graph And Node Model

## Node Class Family (`Core/Node/Base/Node.h`)

- `Node`: base metadata/state
  - `node_id`, `name`, `valid`
  - last timestamps (`streamer`, `order_gateway`, `capture_server`)
- `Producer`: `on_event(Event*)`
- `Consumer`: dirty-state protocol, `update() -> bool`
- `ProducerConsumer`: combined behavior
- `SingleInputConsumer`:
  - one `parent` pointer
  - `compute()` recalculates `value` and sets `valid`
  - `update()` checks parent validity first, then recomputes

## Graph Structure (`Core/Graph/Graph.h/.cpp`)

Core containers:
- `producer_container: map<int, Producer*>`
- `consumer_container: map<int, Consumer*>`
- `adjacency_map: map<int, vector<int>>`
- `update_path: map<int, vector<int>>`

Construction:
- `add_producer(Producer*)`
- `add_edge(Node* publisher, Consumer* subscriber)`
- `resolve_update_path()`

## Update Algorithm

When `graph.update(source_id)` is called:
1. Mark direct children of source as dirty.
2. Iterate precomputed `update_path[source_id]`.
3. For each dirty consumer:
   - call `update()`
   - if it changed, mark its children dirty
   - clear dirty flag

This avoids recomputing unrelated subgraphs.

## Market Producer: `MarketOrderBook`

State:
- bid/ask ladders (`map`-based)
- depth and tick metadata

Event handlers:
- `handle(SnapshotEvent)`: snapshot-to-ladder, validate book
- `handle(OrderEvent)`: apply add/cancel/trade match logic
- `handle(UpdateEvent)`: apply level update
- `handle(HeartBeatEvent)`: staleness check

Book validity rules (`check_snapshot()`):
- both sides non-empty
- best bid/ask prices and sizes non-zero
- best bid < best ask

## Signals

Order book signals (`Core/Node/Signals/OrderBookSignal.cpp`):
- `Mid`
- `Bary`
- `Vwap`
- `TopOfBookImbalance`

Output/debug signal:
- `Print` (`Core/Node/Signals/MathSignal.cpp`) reads parent value and prints.

