# Class: Vwap

Defined in: `Core/Node/Signals/OrderBookSignal.h/.cpp`  
Inherits: `MarketConsumer`

## Role

Computes two-sided VWAP over a target size and returns their midpoint.

## Behavior

- Walks ask ladder to compute ask VWAP for target `size`
- Walks bid ladder to compute bid VWAP for target `size`
- Node is invalid if either side has insufficient volume

## Key API

- constructor: `(instrument, exchange, size)`
- `recompute()`
- `get_value()`, `get_ask_vwap()`, `get_bid_vwap()`

