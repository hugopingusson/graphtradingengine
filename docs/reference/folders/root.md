# Folder: Project Root

Path: `graphtradingengine/`

## Purpose

Top-level entry for build configuration, executable entry points, and source domains (`Core`, `Data`, `Helper`, `Logger`, `tools`).

## Key Files

- `CMakeLists.txt`: defines `GraphTradingEngine` and `MarketDataConverter` targets.
- `main.cpp`: minimal runtime example wiring `Logger`, `Graph`, signal node, and `LiveEngine`.
- `docs/`: architecture and reference documentation.

## Runtime Role

Builds:

- main trading engine executable (`GraphTradingEngine`)
- offline parquet-to-bin conversion executable (`MarketDataConverter`)

