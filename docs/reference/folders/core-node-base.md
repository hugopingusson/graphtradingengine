# Folder: Core/Node/Base

Path: `Core/Node/Base/`

## Purpose

Base node model and core market state producer.

## Files

- `Node.h/.cpp`: `Node`, `Producer`, `Consumer`, `ProducerConsumer`, `Quote`, `MarketConsumer`.
- `MarketNode.h/.cpp`: `Market` producer with fixed-depth order book and event handlers.
- `HeartBeat.h/.cpp`: `HeartBeat` producer (deprecated producer class; heartbeat events remain relevant).

## Runtime Role

Defines the foundational interfaces and the canonical market book state updated by events.

