# Class: BitmexLiveOrderBookStreamer

Defined in: `Core/Streamer/BitmexLiveOrderBookStreamer.h/.cpp`  
Inherits: `LiveUpdateDeltaOrderBookStreamer`

## Role

Live BitMEX websocket L2_25 streamer.

## Feed Strategy

- Subscribes to `orderBookL2_25:<instrument>`
- `partial` action -> builds snapshot -> emits `SnapshotEvent`
- `insert/update/delete` actions -> builds vector of updates -> emits `UpdateBatchEvent`

## Internal State

- `levels_by_id`: map from BitMEX level ID to latest normalized `Update`

