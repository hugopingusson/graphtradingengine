# GraphTradingEngine Documentation

This folder is the canonical project context for humans and LLM agents.

The engine is an event-based trading runtime:
- market/heartbeat events are produced by streamers,
- events update producer nodes,
- the graph propagates updates through dependent consumer nodes.

## Reading Order

For a first pass:
1. `architecture/project-overview.md`
2. `architecture/event-and-data-model.md`
3. `architecture/graph-and-node-model.md`
4. `engines/backtest.md`
5. `engines/live.md`
6. `streamers/overview.md`
7. `streamers/exchanges.md`
8. `data/storage-and-converters.md`
9. `data/configuration.md`
10. `development/extension-playbook.md`

For LLM ingestion first:
1. `llm/context.yaml`
2. `architecture/*.md`
3. `engines/*.md`

## Repository Scope

Main modules:
- `Core/Graph`: `Event`, `Graph`
- `Core/Node/Base`: base node classes, `MarketOrderBook`, `HeartBeat`
- `Core/Node/Signals`: order book signals and `Print`
- `Core/Streamer`: backtest and live streamers
- `Core/Engine`: `BacktestEngine`, `LiveEngine`
- `Data/DataStructure`: in-memory and on-disk message structs
- `Data/DataReader`: parquet to bin converters
- `Helper`: time, db routing, futures roll, config (`Saphir`)
- `Logger`: async file logger wrapper

