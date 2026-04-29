# Folder: Core/Node/Signals

Path: `Core/Node/Signals/`

## Purpose

Signal computation nodes that consume `Market` parent state.

## Files

- `OrderBookSignal.h/.cpp`: `Mid`, `Bary`, `Vwap`, `TopOfBookImbalance`
- `MathSignal.h`: placeholder for additional signal composition utilities

## Runtime Role

Transforms top-of-book and ladder state into scalar features for strategy and model layers.

