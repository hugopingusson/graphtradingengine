# Exchange Streamers

## BitMEX (`BitmexLiveOrderBookStreamer`)

Source:
- host `www.bitmex.com`
- websocket topic `orderBookL2_25:<instrument>`

Behavior:
- `partial` message builds a full snapshot cache, emits `SnapshotEvent`.
- `insert/update/delete` messages emit `UpdateEvent`.
- Internal map `levels_by_id` tracks level ids from BitMEX feed.

Timestamp handling:
- parse ISO8601 feed timestamps when available
- fallback to local `now_ns()`

## Binance (`BinanceLiveOrderBookStreamer`)

Source:
- host `stream.binance.com:9443`
- stream `/ws/<instrument_lower>@depth20@100ms`

Behavior:
- accepts both `b/a` and `bids/asks` payload forms
- reconstructs ladders per message
- emits `SnapshotEvent` per payload

Timestamp handling:
- uses event field `E` when present
- heuristic conversion of seconds/ms/us/ns to nanoseconds

## Deribit (`DeribitLiveOrderBookStreamer`)

Source:
- host `www.deribit.com`
- channel `book.<instrument>.none.10.100ms`

Behavior:
- `type=snapshot` initializes internal ladders, emits `SnapshotEvent`.
- subsequent updates emit `UpdateEvent`.
- supports action-aware levels (`new`, `change`, `delete`) and compact levels.

## OKX (`OkxLiveOrderBookStreamer`)

Source:
- host `ws.okx.com:8443`
- public channel `books5`

Behavior:
- decodes `bids`/`asks` arrays from `data`
- emits `SnapshotEvent` per data payload

Timestamp handling:
- uses `ts` when present
- same unit heuristic as other streamers

