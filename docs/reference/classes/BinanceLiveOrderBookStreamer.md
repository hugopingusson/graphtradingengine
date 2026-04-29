# Class: BinanceLiveOrderBookStreamer

Defined in: `Core/Streamer/BinanceLiveOrderBookStreamer.h/.cpp`  
Inherits: `LiveSnapshotOrderBookStreamer`

## Role

Live Binance websocket depth streamer.

## Feed Strategy

- Connects to `/<symbol>@depth20@100ms`
- Accepts either:
  - diff-style arrays (`b`, `a`)
  - partial-style arrays (`bids`, `asks`)
- Normalizes payload into snapshot ladders and emits `SnapshotEvent`

## Notes

Uses safe numeric parsing from JSON numbers or numeric strings.

