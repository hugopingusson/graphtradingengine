# Streamer Architecture

## Base Classes

Backtest side (`Core/Streamer/Streamer.h`):
- `BacktestStreamer` interface
- `DatabaseWMBPBacktestStreamer`
- `HeartBeatBackTestStreamer`
- `BackTestStreamerContainer`

Live side (`Core/Streamer/LiveStreamer.h`):
- `LiveStreamer`
- `LiveOrderBookStreamer`
- `LiveBootstrappedOrderBookStreamer`
- `LiveOrderDeltaOrderBookStreamer`
- `LiveUpdateDeltaOrderBookStreamer`
- `LiveSnapshotOrderBookStreamer`

## Ring Buffer

`SpscRingBuffer<T>`:
- single-producer single-consumer lock-free pattern
- power-of-two capacity
- atomic `head`/`tail`
- methods: `push`, `pop`, `peek`, `size`, `empty`

Used as:
- one ring per live streamer thread (producer)
- `LiveEngine` consumer loop as single consumer

## Live Event Emission Helpers

`LiveOrderBookStreamer` provides:
- `emit_mbp_event(...)` -> `SnapshotEvent`
- `emit_mbo_event(...)` -> `OrderEvent`
- `emit_update_event(...)` -> `UpdateEvent`

These methods set instrument and source id consistently.

## Bootstrap Pattern

For delta feeds (`LiveBootstrappedOrderBookStreamer`):
- initial snapshot must arrive first
- then delta/update events are accepted
- bootstrap state tracked by `bootstrapped_`

