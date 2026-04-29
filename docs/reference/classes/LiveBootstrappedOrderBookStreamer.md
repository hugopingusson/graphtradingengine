# Class: LiveBootstrappedOrderBookStreamer

Defined in: `Core/Streamer/LiveStreamer.h/.cpp`  
Inherits: `LiveOrderBookStreamer`

## Role

Adds bootstrap state for feeds requiring an initial snapshot before deltas are trusted.

## Key API

- `is_bootstrapped()`
- `mark_bootstrapped()`
- `reset_bootstrap()`

## Behavior

On ring overflow, bootstrap state is reset and reconnect is requested through base overflow handling.

