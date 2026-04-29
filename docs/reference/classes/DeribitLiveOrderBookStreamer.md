# Class: DeribitLiveOrderBookStreamer

Defined in: `Core/Streamer/DeribitLiveOrderBookStreamer.h/.cpp`  
Inherits: `LiveUpdateDeltaOrderBookStreamer`

## Role

Live Deribit websocket order book streamer.

## Feed Strategy

- Subscribes to `book.<instrument>.none.10.100ms`
- `type=snapshot` -> rebuild ladders -> emit `SnapshotEvent` and bootstrap
- change updates -> apply parsed levels to ladders and emit per-level `UpdateEvent`

## Internal State

- `bid_ladder` / `ask_ladder` maintained incrementally between payloads

