# Class Template: SpscRingBuffer<T>

Defined in: `Core/Streamer/LiveStreamer.h`

## Role

Lock-free single-producer/single-consumer ring buffer used by live streamers.

## Key Properties

- Capacity rounded to power-of-two
- Non-blocking `push`/`pop`/`peek`
- Atomics for head/tail indices

## Usage in Project

`LiveStreamer` stores `SpscRingBuffer<std::unique_ptr<Event>>` for event handoff from streamer thread to engine consumer loop.

