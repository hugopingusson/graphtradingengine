# Class: FutureHelper

Defined in: `Helper/FutureHelper.h/.cpp`

## Role

Resolves liquid futures contract code for a date.

## Key Responsibilities

- Provide roll dates by schema (`quarterly`, `monthly`)
- Select active contract month from date vs roll dates
- Build contract symbol like `<base><month_code><year_digit>`

## Main Methods

- `static get_roll_date(year, schema)`
- `get_liquid_contract(date)`

