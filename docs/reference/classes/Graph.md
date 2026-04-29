# Class: Graph

Defined in: `Core/Graph/Graph.h/.cpp`

## Role

Owns graph topology and executes incremental consumer updates after producer changes.

## Key Responsibilities

- Store node containers and adjacency
- Assign node IDs on registration
- Build per-source `update_path` topological execution order
- Dirty-propagate updates through consumer chain
- Own producer lifetime (deletes producers in destructor)

## Main Methods

- Construction: `add_producer`, `add_edge`
- Scheduling: `resolve_update_path`
- Runtime: `update(const int& source_id)`

