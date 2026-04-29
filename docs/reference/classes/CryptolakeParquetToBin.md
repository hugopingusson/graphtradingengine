# Class: CryptolakeParquetToBin

Defined in: `Data/DataReader/CryptolakeParquetToBin.h/.cpp`

## Role

Converts Cryptolake order book parquet files to `MarketByPriceEventPod`.

## Output Layout

Mirrors to:

`<bin_root>/nyk/cryptolake/<instrument>/<date>.bin`

## Main Methods

- `convert_all_to_bin()`
- `convert_to_bin(symbol, exchange, date)`
- `convert_file_to_bin(parquet_path, bin_path)`

