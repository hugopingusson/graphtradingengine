# Folder: Data/DataStructure

Path: `Data/DataStructure/`

## Purpose

Defines canonical engine data structures, enums, ladders, snapshots, and POD serialization records.

## Core Elements

- Enums: `Action`, `Side`, `Location`, `Listener`
- Market timestamp and level structures: `MarketTimeStamp`, `BookLevel`, `SnapshotData`
- Message wrappers: `OrderMessage`, `UpdateMessage`, `SnapshotMessage`, `MarketByPriceMessage`
- On-disk record: `MarketByPriceEventPod`
- Merge helper: `HeapItem`

## Runtime Role

Single shared data contract across streamers, events, market node, and backtest storage.

