# Folder: Core

Path: `Core/`

## Purpose

Contains the trading runtime: engines, event graph, nodes, and streamers.

## Subfolders

- `Engine`: orchestrators (`BacktestEngine`, `LiveEngine`)
- `Graph`: event types and graph propagation logic
- `Node`: market state producers and signal consumers
- `Streamer`: historical and live market data ingestion

## Runtime Role

Implements the end-to-end data path:

`streamer -> event -> producer(node) -> graph update -> consumer signals`

