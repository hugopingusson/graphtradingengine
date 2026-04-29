# Class: TopOfBookImbalance

Defined in: `Core/Node/Signals/OrderBookSignal.h/.cpp`  
Inherits: `MarketConsumer`

## Role

Computes top-of-book size imbalance.

## Formula

`(best_bid_size - best_ask_size) / (best_bid_size + best_ask_size)`

## Key API

- constructor: `(instrument, exchange)`
- `recompute()`
- `get_value()`

