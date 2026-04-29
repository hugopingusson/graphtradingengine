# Class: BackTestStreamerContainer

Defined in: `Core/Streamer/Streamer.h/.cpp`

## Role

Owns and routes all backtest streamers required by graph producers.

## Key Responsibilities

- Register producer sources and create/reuse compatible streamers
- Set producer node IDs on streamers
- Route all streamers to date window before backtest run
- Own streamer lifetime (deletes raw pointers in destructor)

## Main Methods

- `register_source(Producer*)`
- `register_market_source(Market*)`
- `route_and_set_streamers(start, end)`

