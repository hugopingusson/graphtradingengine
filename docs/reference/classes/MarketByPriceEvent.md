# Class: MarketByPriceEvent

Defined in: `Core/Graph/Event.h/.cpp`  
Inherits: `MarketEvent`

## Role

Carries `MarketByPriceMessage` (snapshot + order) in one event.

## Key API

- `get_message()`
- `get_snapshot_data()`
- `get_order()`
- `get_last_market_timestamp()`

## Usage

Primary backtest event type emitted from `MarketByPriceBacktestStreamer`.

