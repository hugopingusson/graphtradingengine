# Folder: Logger

Path: `Logger/`

## Purpose

Asynchronous structured logging wrapper around `spdlog`.

## Files

- `Logger.h/.cpp`: `Logger` class and `LoggerMode` (`DEBUG`, `LIVE`).

## Runtime Role

- Provides component-scoped logging (`log_info`, `log_warn`, `log_error`).
- Applies flush policy by mode:
  - `DEBUG`: flush on `info`
  - `LIVE`: flush on `error`

