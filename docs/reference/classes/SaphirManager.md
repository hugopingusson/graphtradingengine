# Class: SaphirManager

Defined in: `Helper/SaphirManager.h/.cpp`

## Role

Configuration manager for the `~/Saphir` JSON config set.

## Managed Config Files

- `DatabaseConfig.json`
- `InstrumentConfig.json`
- `EngineConfig.json`
- `TickConfig.json`

## Key Responsibilities

- Ensure config files exist with defaults
- Read/write exchange database roots
- Read instrument sets (`unordered_set`)
- Read engine parameters (`market_depth`, `ring_capacity`, `max_update_batch_size`, `logger_mode`)
- Read per-exchange/per-instrument `tick_value`

