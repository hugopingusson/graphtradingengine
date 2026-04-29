# Class: UpdateEvent

Defined in: `Core/Graph/Event.h/.cpp`  
Inherits: `MarketEvent`

## Role

Carries one order-book level update (`UpdateMessage`).

## Key API

- `get_message()`
- `get_update()`
- `get_last_market_timestamp()`

## Usage

Used by live delta streamers when a single update is emitted.

