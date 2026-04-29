# Class: LiveStreamer

Defined in: `Core/Streamer/LiveStreamer.h/.cpp`  
Inherits: `Streamer`

## Role

Base live streamer with thread lifecycle and event ring transport.

## Key Responsibilities

- Manage worker thread (`start`, `stop`, `join`)
- Push parsed events into SPSC ring
- Track pushed/dropped event counters
- Expose reconnect/desync signals to derived classes

## Extension Point

- `virtual void run_loop() = 0`
- optional `on_ring_overflow()`

