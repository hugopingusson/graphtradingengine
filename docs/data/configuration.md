# Configuration (Saphir)

## Location

Managed by `Helper/SaphirManager.h/.cpp`.

Default root:
- `~/Saphir`

Generated files:
- `~/Saphir/DatabaseConfig.json`
- `~/Saphir/InstrumentConfig.json`

## DatabaseConfig.json

Purpose:
- map exchange -> database root path

Current supported exchanges in `SaphirManager`:
- `cme`
- `binance`
- `okx`

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
