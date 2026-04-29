# Class: Node

Defined in: `Core/Node/Base/Node.h/.cpp`

## Role

Base class for all graph vertices.

## Core State

- identity: `node_id`, `name`
- validity: `valid`
- timing: `last_reception_timestamp`, `last_order_gateway_in_timestamp`
- logging: `logger`

## Notes

`Producer` and `Consumer` both derive virtually from `Node`.

