# Class: SaphirManager

Defined in: `Helper/SaphirManager.h/.cpp`

## Role

Configuration manager for the `~/Saphir` JSON config set.

## Managed Config Files

- `DatabaseConfig.json`
- `InstrumentConfig.json`
- `LiveEngineConfig.json`
- `TickConfig.json`

## Key Responsibilities

- Ensure config files exist with defaults
- Read/write exchange database roots
- Read instrument sets (`unordered_set`)
- Read live engine parameters (`market_depth_by_exchange`, `ring_capacity`, `max_update_batch_size`, `logger_mode`)
- Read and validate `supported_live_exchange` for live startup policy
- Read per-exchange/per-instrument `tick_value`

## Exchange Validation

- `normalize_exchange(...)` canonicalizes input to lowercase and validates known exchanges.
- Current known exchanges: `cme`, `binance`, `okx`, `bitmex`, `deribit`.
- `supported_live_exchange` is stricter than known exchanges; `cme` must not be listed there.
