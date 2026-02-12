#pragma once
#include <unordered_set>
#include <unordered_map>
#include <string>
#include "placement.hpp"
#include "artwork.hpp"
#include "hash_utils.hpp"

// Key for (room, artist) for the "no duplicate artist in same room" constraint.
struct RoomArtistKey {
    std::uint16_t room = 0;
    std::string artist;

    bool operator==(const RoomArtistKey& o) const {
        return room == o.room && artist == o.artist;
    }
};

struct RoomArtistHash {
    std::size_t operator()(const RoomArtistKey& k) const {
        std::size_t seed = 0;
        hash_combine(seed, hash_u64(k.room));
        hash_combine(seed, hash_one(k.artist));
        return seed;
    }
};

// Constraint checker skeleton.
// Add/remove placements and check constraints in O(1) expected time using hash tables.
class ConstraintChecker {
public:
    // Example constraints (edit as needed):
    // 1) A wall can hold at most one artwork
    // 2) No two artworks by same artist in same room
    // 3) At most max_sculptures sculptures in a room

    explicit ConstraintChecker(int max_sculptures_per_room = 3)
        : max_sculptures(max_sculptures_per_room) {}

    // Returns true if placement is allowed (given current state).
    bool can_place(const Placement& p, const Artwork& art) const;

    // Apply placement to internal state (assumes it is allowed).
    void place(const Placement& p, const Artwork& art);

private:
    int max_sculptures = 3;

    // 1) occupied locations
    std::unordered_set<Location, LocationHash> occupied;

    // 2) (room, artist) membership
    std::unordered_set<RoomArtistKey, RoomArtistHash> room_artist_used;

    // 3) sculpture count per room
    std::unordered_map<std::uint16_t, int> sculptures_in_room;
};
