# Class: ParquetDataReader

Defined in: `Data/DataReader/ParquetDataReader.h/.cpp`

## Role

Thin Arrow/Parquet table reader for CME market data parquet files.

## Key Responsibility

- Resolve liquid futures contract from date (`FutureHelper`)
- Build parquet path under configured root
- Return loaded `arrow::Table`

## Main Method

- `get_cme_market_data_table(ticker, date)`

