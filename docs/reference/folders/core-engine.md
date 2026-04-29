# Folder: Core/Engine

Path: `Core/Engine/`

## Purpose

Engine orchestrators for backtest and live execution.

## Files

- `BacktestEngine.h/.cpp`: deterministic replay using timestamp-ordered backtest streamers.
- `LiveEngine.h/.cpp`: live consumer loop that merges multiple streamer rings by event reception timestamp.

## Runtime Role

- Initializes graph update paths.
- Builds streamers from graph producers.
- Drives the event loop and calls `Graph::update(source_id)` after producer state updates.

