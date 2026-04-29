# GraphTradingEngine Reference

This section is a code-level reference intended for both developers and LLM context loading.

It is split into:

- `folders/`: one `.md` per source folder
- `classes/`: one `.md` per C++ class declared in project headers

## Folder Reference

- [Project Root](folders/root.md)
- [Core](folders/core.md)
- [Core/Engine](folders/core-engine.md)
- [Core/Graph](folders/core-graph.md)
- [Core/Node](folders/core-node.md)
- [Core/Node/Base](folders/core-node-base.md)
- [Core/Node/Signals](folders/core-node-signals.md)
- [Core/Streamer](folders/core-streamer.md)
- [Data](folders/data.md)
- [Data/DataReader](folders/data-datareader.md)
- [Data/DataStructure](folders/data-datastructure.md)
- [Helper](folders/helper.md)
- [Logger](folders/logger.md)
- [tools](folders/tools.md)

## Class Reference

All class-level docs are in `docs/reference/classes/`.

Main groups:

- Graph and events: `Graph`, `Event`, `MarketEvent` hierarchy
- Nodes: `Node`, `Producer`, `Consumer`, `Market`, `MarketConsumer`, order-book signals
- Streamers: backtest streamers, live streamers, exchange-specific implementations
- Engines: `BacktestEngine`, `LiveEngine`
- Data and helpers: converters, time/config helpers, logger

