# Class: Mid

Defined in: `Core/Node/Signals/OrderBookSignal.h/.cpp`  
Inherits: `MarketConsumer`

## Role

Computes mid price from market best bid/ask.

## Formula

`mid = (best_bid + best_ask) / 2`

## Key API

- constructor: `(instrument, exchange)`
- `recompute()`
- `get_value()`

