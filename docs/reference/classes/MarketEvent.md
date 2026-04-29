# Class: MarketEvent

Defined in: `Core/Graph/Event.h/.cpp`  
Inherits: `Event`

## Role

Abstract base event for market-data events bound to one `instrument`.

## Key API

- `get_instrument()`
- `virtual MarketTimeStamp get_last_market_timestamp() const = 0`
- `dispatchTo(Market&)`

## Notes

Concrete subclasses carry snapshot, order, update, update-batch, or trade payloads.

