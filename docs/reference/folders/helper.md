# Folder: Helper

Path: `Helper/`

## Purpose

Support utilities for time handling, futures contract resolution, database pathing, and JSON config management.

## Files

- `SaphirManager.*`: config root and JSON accessors (`DatabaseConfig`, `InstrumentConfig`, `EngineConfig`, `TickConfig`).
- `TimeHelper.*`: `Date`, `Time`, `Timestamp` utilities with seconds/nanoseconds support.
- `FutureHelper.*`: liquid contract resolution from date and roll schema.
- `DataBaseHelper.*`: resolves historical `.bin` file path from date/instrument/exchange.
- `ToolBox.*`, `Configuration.h`: utility placeholders/shared constants area.

## Runtime Role

Holds environment-dependent configuration and utility logic used across engine paths.

