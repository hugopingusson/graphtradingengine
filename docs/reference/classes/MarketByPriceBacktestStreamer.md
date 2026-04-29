# Class: MarketByPriceBacktestStreamer

Defined in: `Core/Streamer/Streamer.h/.cpp`  
Inherits: `BacktestStreamer`, `MarketStreamer`

## Role

Reads `MarketByPriceEventPod` records from `.bin` and emits `MarketByPriceEvent` to graph.

## Key Responsibilities

- Resolve file path from date/instrument/exchange via `DataBaseHelper`
- Seek first row at/after backtest start timestamp
- Keep current row in memory for heap ordering
- Process row into producer `on_event` + `graph->update(source_id)`

## Main Methods

- `set_and_route(start, end)`
- `advance()`
- `process_current(Graph*)`

