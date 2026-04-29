# Class: LiveEngine

Defined in: `Core/Engine/LiveEngine.h/.cpp`

## Role

Runs live ingestion and graph updates by consuming events from live streamer ring buffers.

## Key Responsibilities

- Build and attach live streamers from graph producers
- Start/stop/join streamer threads
- Merge per-streamer head events by `reception_timestamp`
- Dispatch events to producer node and trigger `Graph::update(source_id)`

## Main Methods

- `initialize()`
- `build_streamer_container()`
- `run()`, `start()`, `stop()`, `join()`
- `run_consumer_loop()`

