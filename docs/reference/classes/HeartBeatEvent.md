# Class: HeartBeatEvent

Defined in: `Core/Graph/Event.h/.cpp`  
Inherits: `Event`

## Role

Represents heartbeat ticks with a clock frequency payload.

## Key API

- `get_frequency()`
- `dispatchTo(Market&)` -> `Market::handle(HeartBeatEvent&)`

## Notes

Used for staleness checks in `Market`.

