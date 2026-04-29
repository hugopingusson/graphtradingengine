# Class: HeartBeat

Defined in: `Core/Node/Base/HeartBeat.h/.cpp`  
Inherits: `Producer`

## Role

Legacy producer carrying heartbeat frequency and timestamp updates.

## Key API

- constructor with `frequency`
- `get_frequency()`
- `on_event(Event*)` updates local reception timestamp from `HeartBeatEvent`

## Notes

Heartbeat logic is primarily event-driven (`HeartBeatEvent`) in the current architecture.

