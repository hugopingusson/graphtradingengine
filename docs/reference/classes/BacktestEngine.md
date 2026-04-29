# Class: BacktestEngine

Defined in: `Core/Engine/BacktestEngine.h/.cpp`

## Role

Runs historical replay by consuming timestamp-ordered events from backtest streamers.

## Key Responsibilities

- Initialize graph (`resolve_update_path`)
- Build streamer container from graph producers
- Route streamers to date window
- Merge streamer heads via min-heap and process events in time order

## Main Methods

- `initialize()`
- `build_streamer_container()`
- `run(const Timestamp& start, const Timestamp& end)`

