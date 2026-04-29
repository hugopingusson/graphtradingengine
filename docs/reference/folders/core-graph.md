# Folder: Core/Graph

Path: `Core/Graph/`

## Purpose

Defines:

- event model (`Event` hierarchy)
- graph topology and propagation engine (`Graph`)

## Files

- `Event.h/.cpp`: base `Event` plus market/heartbeat/event-specialized subclasses with double-dispatch to `Market`.
- `Graph.h/.cpp`: adjacency storage, update path resolution, and dirty-propagation execution.

## Runtime Role

Acts as computation scheduler for nodes once producer data has changed.

