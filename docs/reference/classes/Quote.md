# Class: Quote

Defined in: `Core/Node/Base/Node.h/.cpp`  
Inherits: `Consumer`

## Role

Abstract quote-like consumer holding `ask_price` and `bid_price`.

## Key API

- `get_ask_price()`, `get_bid_price()`
- derived metrics: `mid()`, `spread()`
- pure virtual `update()`

