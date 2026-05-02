# Live Engine

File: `Core/Engine/LiveEngine.cpp`

## Responsibilities

- Resolve graph update path.
- Build live streamer set from graph producers.
- Start streamer threads.
- Consume per-streamer ring buffers in timestamp order.
- Feed events to producers and trigger graph updates.

## Startup

`LiveEngine::run()`:
1. `initialize()`
2. `start()` streamer threads
3. `run_consumer_loop()` on caller thread
4. `join()` streamer threads on exit

Important: consumer loop runs in the main/caller thread, not in an internal worker thread.

## Streamer Construction

`register_market_source()` builds streamers from `market->get_exchange()`:
- `bitmex` -> `BitmexLiveOrderBookStreamer`
- `binance` -> `BinanceLiveOrderBookStreamer`
- `deribit` -> `DeribitLiveOrderBookStreamer`
- `okx` -> `OkxLiveOrderBookStreamer`

If a streamer for same `(instrument, exchange)` already exists, it is reused and only source id is updated.

Before streamer creation, exchange membership is checked against
`SaphirManager::get_supported_live_exchanges()` (from `LiveEngineConfig.json`).
If not listed, startup fails fast with a runtime error.

## Consumer Loop Ordering

`run_consumer_loop()` uses:
- one SPSC ring per streamer
- a min-heap of current head events from each ring
- comparison key: `event->get_streamer_in_timestamp()`

Logic:
1. Rescan streamers to enqueue head events.
2. Pop smallest candidate from heap.
3. Validate candidate against current ring head timestamp.
4. Pop event from selected streamer.
5. Push selected streamer next head (if any).
6. Process event (`producer->on_event`, then `graph->update(source_id)`).

Idle policy:
- short spinning with `yield()`
- then sleep 50 microseconds after repeated empty cycles

## Stop/Join Semantics

- `stop()` sets engine running flag to false and requests stop on each streamer.
- `join()` waits all streamer threads.
- `run()` is blocking; external shutdown signal/call is needed for continuous live use.
