# Class: LiveOrderBookStreamer

Defined in: `Core/Streamer/LiveStreamer.h/.cpp`  
Inherits: `LiveStreamer`, `MarketStreamer`

## Role

Common base for live order book streamers.

## Key Responsibilities

- Emit normalized internal events:
  - snapshot (`SnapshotEvent`)
  - order (`OrderEvent`)
  - update (`UpdateEvent`)
  - update batch (`UpdateBatchEvent`)
- Chunk large update vectors using `max_update_batch_size`

## Notes

This class is event-format focused; transport/parsing is implemented in exchange subclasses.

