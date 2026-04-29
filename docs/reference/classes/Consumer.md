# Class: Consumer

Defined in: `Core/Node/Base/Node.h/.cpp`  
Inherits: `Node`

## Role

Abstract compute node updated by graph propagation.

## Key Concepts

- Dirty flag: `mark_dirty()`, `clear_dirty()`, `is_dirty()`
- `update()` returns whether node state changed (used for downstream dirty propagation)

