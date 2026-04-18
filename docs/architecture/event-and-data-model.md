# Event And Data Model

## Data Primitives (`Data/DataStructure/DataStructure.h`)

- `kBookLevels = 10`
- `Action`: `ADD`, `CANCEL`, `MODIFY`, `TRADE`
- `Side`: `BID`, `ASK`, `NEUTRAL`
- `MarketTimeStamp`:
  - `order_gateway_in_timestamp`
  - `data_gateway_out_timestamp`

Order book containers:
- `BidLadder`: `std::map<double, BookLevel, std::greater<double>>`
- `AskLadder`: `std::map<double, BookLevel>`
- `SnapshotData`: fixed-size top-of-book arrays (10 levels each side)

Messages:
- `SnapshotMessage`
- `OrderMessage`
- `UpdateMessage`
- `MarketByPriceMessage` (still present; mixed historical usage)

Backtest row format:
- `BacktestWideMarketByPriceRow`
  - `capture_server_in_timestamp`
  - `market_time_stamp`
  - `order_book_snapshot_data`
  - `order`

## Event Hierarchy (`Core/Graph/Event.h`)

Base:
- `Event`
  - `capture_server_in_timestamp`
  - `streamer_in_timestamp`
  - `source_id_trigger`

Derived:
- `HeartBeatEvent`
- `MarketEvent` (adds `instrument`)
  - `SnapshotEvent` (`SnapshotMessage`)
  - `OrderEvent` (`OrderMessage`)
  - `UpdateEvent` (`UpdateMessage`)
  - `TradeEvent` (legacy path, carries side/price/quantity)

## Timestamp Semantics

Event-level timestamps:
- `capture_server_in_timestamp`: capture-side timestamp (historical provider or capture server).
- `streamer_in_timestamp`: time when streamer injected event into engine pipeline.

Message-level timestamps (`MarketTimeStamp`):
- `order_gateway_in_timestamp`: market/exchange-side ingress timestamp.
- `data_gateway_out_timestamp`: upstream distribution egress timestamp.

## Dispatch Model

`Event` uses double dispatch:
- Engine calls `producer->on_event(event)`.
- `MarketOrderBook::on_event` calls `event->dispatchTo(*this)`.
- Concrete `handle(...)` overload is selected by event type.

