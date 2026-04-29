# Class: OrderEvent

Defined in: `Core/Graph/Event.h/.cpp`  
Inherits: `MarketEvent`

## Role

Carries one order-style message (`OrderMessage`) for by-order feeds.

## Key API

- `get_message()`
- `get_order()`
- `get_last_market_timestamp()`

## Usage

Used when a venue feed is naturally order-add/cancel/trade based.

