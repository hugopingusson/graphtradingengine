# Class: Logger

Defined in: `Logger/Logger.h/.cpp`

## Role

Asynchronous logging facade using `spdlog`.

## Key Responsibilities

- Create log folder/file
- Emit component-scoped info/warn/error entries
- Flush behavior based on `LoggerMode`
- Integrate default mode from `SaphirManager` (`live` or `debug`)

## Main Methods

- `log_info`, `log_warn`, `log_error`
- `flush()`
- `throw_error(component, msg)`

