# Project 3: Randomized Room Assignment & Load Balancing (Balls into Bins)

## Goal
Use hashing as a *randomized assignment mechanism*.

Model:
- n artworks = balls
- m rooms = bins
- bucket size B = max items you want in a room before you call it "overflow"

We assign each artwork to a room using a hash function:
    room = h(artwork_id) mod m

Then we measure:
- load factor alpha = n/m
- maximum room load
- how many rooms overflow (load > B)

## What’s Provided
- `HashedAssigner` implementing:
  - h(x) = (A*x + B) mod P   (universal-hash style)
  - room = h(x) mod m
- chaining-style buckets: `vector<vector<id>>`
- `main.cpp` that prints loads and summary stats

## Build & Run
    make
    ./03_load_balance data/small_artworks.txt 3 2
    ./03_load_balance data/large_artworks.txt 10 6

## Tasks (What Students Add)
### A. Experiments
Run multiple trials by changing A and B constants.
Record:
- max load
- overflow count
for each run.

### B. Compare Good Hash vs Bad Hash
Add a "bad hash" mode, e.g.:
- room = artwork_id % m
or
- room = (artwork_id / 10) % m
Show how structure in IDs can cause imbalance.

### C. Vary alpha and bucket size
Try:
- m fixed, increase n -> alpha increases
- B fixed vs increasing B

Connect to theory:
- expected load per bin is alpha
- overflow probability decreases quickly as B grows (for fixed alpha)

## Collision Notes
Here, collisions are *expected* (many artworks in the same room).
We resolve them by chaining:
- each room has a list/vector of artworks

## Deliverable
- A short write-up with:
  - experiment results table
  - comparison of hash functions
  - conclusions about alpha and overflow

