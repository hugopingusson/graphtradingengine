# Class: MarketStreamer

Defined in: `Core/Streamer/Streamer.h/.cpp`

## Role

Mixin carrying market identity for a streamer.

## Fields

- `instrument`
- `exchange`
- `order_book_source_node_id` (graph producer node ID to trigger)

## Key API

- `get_instrument()`, `get_exchange()`
- `set_order_book_source_node_id()`, `get_order_book_source_node_id()`

