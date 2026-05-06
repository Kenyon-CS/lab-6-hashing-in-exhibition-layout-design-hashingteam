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

// Key for (room, medium) to cap how many artworks of the same medium can
// appear in one room.
struct RoomMediumKey {
    std::uint16_t room = 0;
    std::string medium;

    bool operator==(const RoomMediumKey& o) const {
        return room == o.room && medium == o.medium;
    }
};

struct RoomMediumHash {
    std::size_t operator()(const RoomMediumKey& k) const {
        std::size_t seed = 0;
        hash_combine(seed, hash_u64(k.room));
        hash_combine(seed, hash_one(k.medium));
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

    explicit ConstraintChecker(int max_sculptures_per_room = 3,
                               int max_same_medium_per_room = 2);

    // Returns true if placement is allowed (given current state).
    bool can_place(const Placement& p, const Artwork& art) const;

    // Apply placement to internal state (assumes it is allowed).
    void place(const Placement& p, const Artwork& art);

    // Remove placement from internal state for backtracking search.
    void unplace(const Placement& p, const Artwork& art);

private:
    int max_sculptures = 3;
    int max_same_medium = 2;

    // 1) occupied locations
    std::unordered_set<Location, LocationHash> occupied;

    // 2) (room, artist) membership
    std::unordered_set<RoomArtistKey, RoomArtistHash> room_artist_used;

    // 3) sculpture count per room
    std::unordered_map<std::uint16_t, int> sculptures_in_room;

    // 4) medium count per room
    std::unordered_map<RoomMediumKey, int, RoomMediumHash> medium_count_in_room;

    // 5) room-local wall conflict pairs (for example, adjacent walls that
    // cannot both be occupied at the same time).
    std::unordered_map<Location, Location, LocationHash> conflicting_walls;

    bool is_allowed_wall_for_medium(const Placement& p, const Artwork& art) const;
};
