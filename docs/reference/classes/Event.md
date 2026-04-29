# Class: Event

Defined in: `Core/Graph/Event.h/.cpp`

## Role

Abstract base event for all data entering the graph.

## Fields

- `reception_timestamp`
- `source_id_trigger`
- `location`
- `listener`

## Key API

- Accessors for all base fields
- `virtual void dispatchTo(Market& target) = 0` (double-dispatch entry point)

