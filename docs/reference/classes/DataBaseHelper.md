# Class: DataBaseHelper

Defined in: `Helper/DataBaseHelper.h/.cpp`

## Role

Resolves historical `.bin` file paths from `(date, instrument, exchange)`.

## Key Behavior

- Uses `SaphirManager` for exchange root lookup
- For futures instruments, resolves current liquid contract via `FutureHelper`
- Returns:
  - futures: `<exchange_root>/<liquid_contract>/<date>.bin`
  - spot/crypto: `<exchange_root>/<instrument>/<date>.bin`

