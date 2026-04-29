# Folder: Data/DataReader

Path: `Data/DataReader/`

## Purpose

Historical data ingestion and conversion from parquet to engine `.bin`.

## Files

- `ParquetDataReader.*`: direct Arrow/Parquet table read helper.
- `CMEParquetToBin.*`: converts Databento CME MBP parquet to `MarketByPriceEventPod` bins.
- `CryptolakeParquetToBin.*`: converts Cryptolake order book parquet to `MarketByPriceEventPod` bins.

## Runtime Role

Produces the binary mirror database used by backtest streamers.

