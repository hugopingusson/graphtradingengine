# Class: Market

Defined in: `Core/Node/Base/MarketNode.h/.cpp`  
Inherits: `Producer`

## Role

Canonical order book state node used as parent for most signals.

## Key Responsibilities

- Maintain fixed-size bid/ask ladders (`kBookLevels`, trimmed by configured depth)
- Handle event types via double-dispatch:
  - `MarketByPriceEvent`, `SnapshotEvent`
  - `OrderEvent`, `UpdateEvent`, `UpdateBatchEvent`
  - `HeartBeatEvent` (staleness)
- Expose top-of-book and derived metrics:
  - `mid`, `spread`, `imbalance`, `bary`

## Important Behavior

- Snapshot validation is strict in `debug` logger mode and lean in `live` mode.
- Uses `SaphirManager` for depth configuration and runtime mode.

