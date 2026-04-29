# Class: UpdateBatchEvent

Defined in: `Core/Graph/Event.h/.cpp`  
Inherits: `MarketEvent`

## Role

Carries a vector of `UpdateMessage` produced from one payload (throttled/delta bursts).

## Key API

- `get_messages()`
- `get_batch_size()`
- `get_last_market_timestamp()`

## Usage

Used by exchanges where one websocket payload contains many deltas (for example BitMEX `orderBookL2_25`).

