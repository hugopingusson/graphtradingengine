# Class: Market

Defined in: `Core/Node/Base/MarketNode.h/.cpp`  
Inherits: `Producer`

## Role

Canonical order book state node used as parent for most signals.

## Key Responsibilities

- Maintain bid/ask ladders using `Ladder` (array-backed, max physical depth `kBookLevels`)
- Handle event types via double-dispatch:
  - `MarketByPriceEvent`, `SnapshotEvent`
  - `OrderEvent`, `UpdateEvent`, `UpdateBatchEvent`
  - `HeartBeatEvent` (staleness)
- Expose top-of-book and derived metrics:
  - `mid`, `spread`, `imbalance`, `bary`

## Important Behavior

- Snapshot validation is strict in `debug` logger mode and lean in `live` mode.
- Uses `SaphirManager` for `managed_depth`, `tick_value`, and runtime mode.
- No active trim-by-managed-depth on incremental updates; ladder updates are in-place with per-message shifts.
- `managed_depth` is a validity threshold (`observed_depth >= managed_depth`), not an enforced insert cap.
- `apply_ask_level` / `apply_bid_level` use explicit action branches:
  - `CANCEL`: erase if present
  - `MODIFY`: update only if existing level is found
  - `ADD`: update existing level or insert new level

## Depth Accessors

- `get_observed_ask_depth()` / `get_observed_bid_depth()` return current ladder sizes.
- `get_observed_depth()` returns `min(observed_ask_depth, observed_bid_depth)`.
- `has_required_depth()` centralizes required-depth checks used by event handlers.
