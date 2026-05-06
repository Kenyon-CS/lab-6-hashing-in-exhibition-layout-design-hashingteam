# Project 3: Randomized Room Assignment and Load Balancing

## Hash Functions Compared

### Good hash

We used the provided universal-style hash:

`h(x) = (A*x + B) mod P`, then `room = h(x) mod m`

with `P = 4294967311`, a prime larger than the artwork IDs.

This is a standard randomized assignment strategy because small changes in the
ID tend to spread across many rooms instead of staying clustered.

### Bad hash

We also added a deliberately poor hash:

`room = (artwork_id / 25) mod m`

Because the dataset IDs are sequential, this bad hash sends long consecutive
runs of IDs to the same room. That creates visible hot spots and a less even
distribution.

## Trial Results

### Small dataset (`n = 6`, `m = 3`, `B = 2`)

Good-hash trials:

| A | B | max load | overflow count |
|---|---|----------|----------------|
| 2654435761 | 12345 | 3 | 1 |
| 11400714819323198485 | 97 | 3 | 1 |
| 40503 | 7919 | 6 | 1 |

Good vs bad:

| Mode | max load | overflow count |
|------|----------|----------------|
| universal | 3 | 1 |
| bad-span25 | 6 | 1 |

### Large dataset (`n = 200`, `m = 10`, `B = 6`)

Good-hash trials:

| A | B | max load | overflow count |
|---|---|----------|----------------|
| 2654435761 | 12345 | 22 | 10 |
| 11400714819323198485 | 97 | 22 | 10 |
| 40503 | 7919 | 20 | 10 |

Good vs bad:

| Mode | max load | overflow count |
|------|----------|----------------|
| universal | 22 | 10 |
| bad-span25 | 25 | 8 |

The bad hash produced a **worse maximum load**, showing stronger clustering.
Its overflow count was not always larger because a poor hash can overload a few
rooms while leaving others underused or empty. That is still worse balancing.

## Alpha and Bucket-Size Sweep

Using the main universal hash on the large dataset:

| m | alpha = n/m | B | max load | overflow count |
|---|-------------|---|----------|----------------|
| 5  | 40.00 | 4  | 43 | 5  |
| 5  | 40.00 | 6  | 43 | 5  |
| 5  | 40.00 | 10 | 43 | 5  |
| 10 | 20.00 | 4  | 22 | 10 |
| 10 | 20.00 | 6  | 22 | 10 |
| 10 | 20.00 | 10 | 22 | 10 |
| 20 | 10.00 | 4  | 12 | 20 |
| 20 | 10.00 | 6  | 12 | 20 |
| 20 | 10.00 | 10 | 12 | 5  |

## Conclusions

1. The expected average load per room is `alpha = n/m`, and the measured loads
   stay near that value when the universal hash is used.
2. As `m` increases for fixed `n`, `alpha` drops and the maximum room load also
   drops.
3. Increasing `B` reduces the number of overflow rooms, especially once `B`
   approaches the typical room load.
4. A structured bad hash can create obvious hot spots even when the IDs are
   simple and consecutive, so hash quality matters for balanced assignments.
