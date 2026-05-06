# Project 2: Fast Conflict Checking with Hashing

## Constraints Implemented

The checker keeps all rule checks at O(1) expected time by using hash-based
membership and counting structures.

Implemented constraints:

1. **One artwork per wall/location**
   - Key: `Location`
   - Structure: `unordered_set<Location, LocationHash>`

2. **No duplicate artist in the same room**
   - Key: `(room, artist)`
   - Structure: `unordered_set<RoomArtistKey, RoomArtistHash>`

3. **At most 2 sculptures per room**
   - Key: `room`
   - Structure: `unordered_map<uint16_t, int>`

4. **At most 2 artworks of the same medium per room**
   - Key: `(room, medium)`
   - Structure: `unordered_map<RoomMediumKey, int, RoomMediumHash>`

5. **Wall conflict pairs inside a room**
   - Rule: walls `(1,2)` and `(3,4)` cannot both be occupied in the same room
   - Key: `Location -> conflicting Location`
   - Structure: `unordered_map<Location, Location, LocationHash>`

6. **Sculptures only on designated display spots**
   - Rule: sculptures may only be placed on even-numbered walls/spots
   - Check: arithmetic test on `wall`

## Backtracking Support

`unplace(const Placement&, const Artwork&)` was added so a search algorithm can
undo placements cleanly. It removes:

- the occupied location
- the `(room, artist)` membership
- the sculpture count entry when needed
- the `(room, medium)` count entry when needed

Zero-count map entries are erased to keep the state compact.

## Hash Keys and Collision Handling

All composite keys define both:

- `operator==` for exact equality
- a custom hash functor using `hash_combine(...)`

This means collisions are handled safely by the C++ `unordered_*` containers:
items that land in the same bucket are still distinguished by equality checks,
so collisions do not create false accepts or false rejects.

## Results

### Small dataset

- Attempts processed: `7`
- Accepted: `3`
- Rejected: `4`
- Runtime: about `1.19 ms`

### Large dataset

- Attempts processed: `5000`
- Accepted: `24`
- Rejected: `4976`
- Runtime: about `6.99 ms`

The runtime stays very small even on the larger file because every constraint is
checked with constant-time expected lookup/update work.
