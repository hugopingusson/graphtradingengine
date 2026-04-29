# Folder: Core/Streamer

Path: `Core/Streamer/`

## Purpose

Market data ingestion layer for both backtest and live modes.

## Files

- `Streamer.h/.cpp`: backtest streamer abstractions and container routing.
- `LiveStreamer.h/.cpp`: live streamer base classes + SPSC ring buffer.
- Exchange streamers:
  - `BitmexLiveOrderBookStreamer.*`
  - `BinanceLiveOrderBookStreamer.*`
  - `DeribitLiveOrderBookStreamer.*`
  - `OkxLiveOrderBookStreamer.*`

## Runtime Role

Converts external market data payloads into internal `Event` objects and pushes them into engine consumption flow.

