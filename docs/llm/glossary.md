# Glossary

- Producer: node that ingests events (`on_event`).
- Consumer: node recomputed by graph propagation (`update`).
- SingleInputConsumer: consumer with one parent node and scalar `value`.
- Streamer: component that sources events (live feed or historical file).
- Source ID: integer node id used inside `Event::source_id_trigger`.
- SnapshotEvent: full top-of-book snapshot event.
- OrderEvent: order-style event carrying one `OrderMessage`.
- UpdateEvent: level delta event carrying one `UpdateMessage`.
- Capture Timestamp: timestamp from data capture/provider pipeline.
- Streamer Timestamp: timestamp when streamer pushes event into engine.
- Market Timestamp: exchange/data-gateway timestamps in payload.
- Bootstrapped Streamer: delta streamer that must receive initial snapshot first.
- WMBP Row: `BacktestWideMarketByPriceRow` binary backtest record.

