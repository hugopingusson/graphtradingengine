# Configuration (Saphir)

## Location

Managed by `Helper/SaphirManager.h/.cpp`.

Default root:
- `~/Saphir`

Generated files:
- `~/Saphir/DatabaseConfig.json`
- `~/Saphir/InstrumentConfig.json`
- `~/Saphir/LiveEngineConfig.json`
- `~/Saphir/TickConfig.json`

## DatabaseConfig.json

Purpose:
- map exchange -> database root path

Current supported exchanges in `SaphirManager`:
- `cme`
- `binance`
- `okx`
- `bitmex`
- `deribit`

Default generated example:
```json
{
  "databases": {
    "cme": "/media/hugo/T7/market_data_bin/databento/mbp10",
    "binance": "/media/hugo/T7/market_data_bin/cryptolake/order_book",
    "okx": "/media/hugo/T7/market_data_bin/cryptolake/order_book"
  }
}
```

## InstrumentConfig.json

Purpose:
- list futures and crypto instruments.
- consumed as `unordered_set<string>` in code.

Default generated shape:
```json
{
  "futures": ["6E", "..."],
  "cryptocurrencies": ["BTCUSDT", "..."]
}
```

## LiveEngineConfig.json

Purpose:
- live engine runtime parameters and depth by exchange
- explicit allow-list for live-capable exchanges

Current shape:
```json
{
  "market_depth_by_exchange": {
    "cme": 25,
    "binance": 25,
    "okx": 25,
    "bitmex": 25,
    "deribit": 25
  },
  "supported_live_exchange": ["binance", "okx", "bitmex", "deribit"],
  "ring_capacity": 16384,
  "max_update_batch_size": 64,
  "logger_mode": "live"
}
```

Notes:
- `supported_live_exchange` is enforced by `LiveEngine`.
- `cme` can have depth/tick config for non-live usage, but should not be listed in `supported_live_exchange`.
- On upgrade, `SaphirManager` can bootstrap `LiveEngineConfig.json` from legacy `EngineConfig.json` if present.

## TickConfig.json

Purpose:
- per-exchange, per-instrument tick size lookup

Current behavior:
- lookup key is `tick_values.<exchange>.<instrument>`
- missing or non-positive values throw immediately

## `DataBaseHelper` Path Resolution

`DataBaseHelper::get_data_path(date, instrument, exchange)`:
- loads roots from `SaphirManager`
- if instrument is a configured future:
  - resolves liquid contract via `FutureHelper`
  - returns `<db_root>/<exchange>/<liquid_contract>/<date>.bin`
- otherwise:
  - returns `<db_root>/<exchange>/<instrument>/<date>.bin`

## `FutureHelper`

Used for futures roll logic:
- maps instrument code to rolling contract by date
- supports quarterly/monthly schema logic
