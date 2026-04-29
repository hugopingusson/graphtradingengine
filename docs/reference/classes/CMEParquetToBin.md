# Class: CMEParquetToBin

Defined in: `Data/DataReader/CMEParquetToBin.h/.cpp`

## Role

Converts Databento CME MBP parquet files to engine-native binary records (`MarketByPriceEventPod`).

## Output Layout

Mirrors to:

`<bin_root>/chi/databento/<instrument>/<date>.bin`

## Main Methods

- `convert_all_to_bin()`
- `convert_to_bin(instrument, date)`
- `convert_file_to_bin(parquet_path, bin_path)`

