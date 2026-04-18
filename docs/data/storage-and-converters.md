# Data Storage And Converters

## Binary Layout Convention

Target layout for mirror databases:
- `<root>_bin/<exchange>/<instrument>/<date>.bin`

Examples:
- `/media/hugo/T7/market_data_bin/databento/mbp10/cme/6EM6/2026-03-04.bin`
- `/media/hugo/T7/market_data_bin/cryptolake/order_book/binance/BTCUSDT/2026-03-04.bin`

## Backtest Row Struct

On-disk rows are packed `BacktestWideMarketByPriceRow`:
- `capture_server_in_timestamp`
- `market_time_stamp`
- `order_book_snapshot_data`
- `order`

Consumer:
- `DatabaseWMBPBacktestStreamer` reads this row type sequentially.

## `CMEParquetToBin`

Files:
- `Data/DataReader/CMEParquetToBin.h/.cpp`

Input:
- Databento CME mbp10 parquet tree.

Output:
- writes `.bin` mirror files under CME output root.

Transforms:
- maps parquet normalized columns into one `BacktestWideMarketByPriceRow` per row.
- fills snapshot arrays level-by-level.
- maps action/side strings to enums.
- computes `data_gateway_out_timestamp` from `ts_recv - ts_in_delta`.

## `CryptolakeParquetToBin`

Files:
- `Data/DataReader/CryptolakeParquetToBin.h/.cpp`

Input:
- Cryptolake order book snapshot parquet tree.

Output:
- writes `.bin` mirror files under exchange/instrument/date.

Transforms:
- keeps top `kBookLevels` (10) levels per side.
- parses string timestamps to nanoseconds.
- fills `order` with neutral/empty placeholder because source has no order events.
- stores snapshots as `BacktestWideMarketByPriceRow`.

## Legacy Reader

`ParquetDataReader` still exists for direct Arrow table loading, but the main backtest pipeline is currently binary-row based.
