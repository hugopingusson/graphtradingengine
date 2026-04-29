# Class: Producer

Defined in: `Core/Node/Base/Node.h/.cpp`  
Inherits: `Node`

## Role

Abstract node that consumes incoming `Event` objects and mutates internal state.

## Key API

- `virtual void on_event(Event* event) = 0`

## Implementations

- `Market`
- `HeartBeat`

