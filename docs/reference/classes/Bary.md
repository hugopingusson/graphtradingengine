# Class: Bary

Defined in: `Core/Node/Signals/OrderBookSignal.h/.cpp`  
Inherits: `MarketConsumer`

## Role

Computes barycentric top-of-book price from `Market::bary()`.

## Formula

`bary = mid + 0.5 * spread * imbalance`

## Key API

- constructor: `(instrument, exchange)`
- `recompute()`
- `get_value()`

