# Class: Timestamp

Defined in: `Helper/TimeHelper.h/.cpp`

## Role

High-level timestamp object combining `Date` and `Time`.

## Construction Modes

- from `Date + Time`
- from string `yyyy-mm-dd HH:MM:SS`
- from Unix integer (auto unit heuristic: s/ms/us/ns)

## Key API

- `unixtime(Resolution)`
- `static now_unix(Resolution)`
- `static unix_to_string(unix)` -> `yyyy-mm-dd HH:MM:SS.NS`
- `get_date()`, `get_time()`

