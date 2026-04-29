# Class: MarketConsumer

Defined in: `Core/Node/Base/Node.h/.cpp`  
Inherits: `Consumer`

## Role

Base class for nodes that consume exactly one `Market` producer.

## Key Responsibilities

- Hold instrument/exchange identity
- `connect(Graph&)`:
  - reuse matching `Market` if found
  - otherwise create and add a new `Market` producer
  - add edge `Market -> this`
- `update()` gates on parent validity then calls `recompute()`

## Extension Point

- `virtual bool recompute() = 0`

