# Recent Changes (May 2026)

This note summarizes the key behavior changes introduced in the latest refactor pass.

## Market + Ladder

- Added dedicated array-backed `Ladder` data structure in:
  - `Data/DataStructure/Ladder.h`
  - `Data/DataStructure/Ladder.cpp`
- `Market` now stores `ask_ladder` and `bid_ladder` as `Ladder` instances.
- Incremental update path does not actively trim by `managed_depth`.
- `apply_ask_level` and `apply_bid_level` were simplified with explicit action branches:
  - `ADD`
  - `MODIFY`
  - `CANCEL`
- `MODIFY` only updates existing levels; it does not insert missing levels.

## Depth Semantics

- `managed_depth` is treated as a required validity threshold.
- Added observed depth accessors on `Market`:
  - `get_observed_ask_depth()`
  - `get_observed_bid_depth()`
  - `get_observed_depth()`
- Added `has_required_depth()` and reused it in market event handlers.

## Live Config + Policy

- Live engine config moved to `LiveEngineConfig.json`.
- Added `supported_live_exchange` policy list in live config.
- Live exchange registration now validates against
  `SaphirManager::get_supported_live_exchanges()`.
- `cme` is allowed as a known exchange for config/depth/tick usage but is not a live-supported venue.

## Exchange Coverage

- `normalize_exchange(...)` supports:
  - `cme`
  - `binance`
  - `okx`
  - `bitmex`
  - `deribit`

## Saphir Defaults and Migration

- `SaphirManager` now ensures/reads `LiveEngineConfig.json`.
- Bootstrap supports migration from legacy `EngineConfig.json` when present.
- Tick config and depth config were expanded to include `bitmex` and `deribit` entries.
