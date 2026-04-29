# Class: OkxLiveOrderBookStreamer

Defined in: `Core/Streamer/OkxLiveOrderBookStreamer.h/.cpp`  
Inherits: `LiveSnapshotOrderBookStreamer`

## Role

Live OKX websocket order book streamer.

## Feed Strategy

- Subscribes to channel `books5`
- Parses `data[]` entries containing bids/asks arrays
- Builds normalized snapshot and emits `SnapshotEvent`

## Notes

Exchange timestamp `ts` is normalized to nanoseconds when available.

