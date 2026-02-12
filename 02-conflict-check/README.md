# Project 2: Fast Conflict Checking with Hashing

## Goal
In the exhibition layout problem, every candidate placement must be checked against constraints:
- "Is this wall already occupied?"
- "Have we already placed an artwork by this artist in this room?"
- "Have we exceeded the sculpture limit in this room?"
- (extensions) lighting, adjacency, walkways, sightlines, etc.

Your job is to build an **O(1) expected-time** constraint checker using hash tables.

## What’s Provided
- Core structs: `Artwork`, `Location`, `Placement`
- Hash helpers: `LocationHash`, `RoomArtistHash`, etc.
- `ConstraintChecker` skeleton using:
  - `unordered_set<Location>` for occupied walls
  - `unordered_set<RoomArtistKey>` for artist-in-room constraint
  - `unordered_map<room, count>` for sculpture caps
- `main.cpp` that loads:
  - artworks from CSV
  - placement attempts from a text file
  - prints ACCEPT/REJECT for each attempt

## Build & Run
    make
    ./02_conflict_check data/small_artworks.csv data/small_attempts.txt
    ./02_conflict_check data/large_artworks.csv data/large_attempts.txt

## Input Formats
### Artworks CSV
    id,artist,medium,title

### Attempts TXT
    room wall artwork orientation

Lines beginning with `#` are ignored.

## Tasks (What Students Add)
### A. Add Constraints (choose 2–3)
Ideas:
- Per-wall capacity (e.g., only 1 painting; sculptures only on certain walls/spots)
- "No more than K artworks of the same medium in a room"
- "These two walls are a conflict pair (cannot both be used)"
- "Keep a minimum distance between sculptures" (model as conflicts between spots)

Implement each constraint so `can_place()` stays O(1) expected time.
That usually means adding another `unordered_set` or `unordered_map`.

### B. Support Backtracking (remove)
If you want to integrate with a search algorithm, add:
- `unplace(Placement, Artwork)`
and ensure your data structures update correctly.

### C. Stress Test Performance
Try scaling attempts to 50,000 or 200,000 by generating larger attempt files.
Measure runtime and show it remains fast.

## Hash Function Notes
You are using C++ `unordered_*` containers, so hash collisions are handled internally
(separate chaining). Still, you must:
- define `operator==` correctly for your key structs
- define stable hashes for composite keys (we provide `hash_combine`)

## Deliverable
- A short write-up describing:
  - constraints implemented
  - hash keys used
  - how collisions/equality are handled
  - results on small + large data

