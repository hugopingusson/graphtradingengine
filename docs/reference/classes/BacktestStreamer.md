# Class: BacktestStreamer

Defined in: `Core/Streamer/Streamer.h/.cpp`  
Inherits: `Streamer`

## Role

Abstract interface for deterministic historical replay streamers.

## Required Methods

- `advance()`
- `is_good()`
- `get_current_heap_item()`
- `set_and_route(start, end)`
- `process_current(Graph* graph)`

