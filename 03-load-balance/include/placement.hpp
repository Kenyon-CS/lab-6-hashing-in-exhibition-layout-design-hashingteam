#pragma once
#include <cstdint>
#include <string>
#include "hash_utils.hpp"

// A "location" is (room_id, wall_id). For sculptures you might treat "floor spots"
// the same way (room_id, spot_id).
struct Location {
    std::uint16_t room = 0; // 1..R
    std::uint16_t wall = 0; // 1..W

    bool operator==(const Location& o) const { return room==o.room && wall==o.wall; }
};

struct LocationHash {
    std::size_t operator()(const Location& L) const {
        std::size_t seed = 0;
        hash_combine(seed, hash_u64(L.room));
        hash_combine(seed, hash_u64(L.wall));
        return seed;
    }
};

// A placement is one artwork at one location with an orientation.
struct Placement {
    Location loc;
    std::uint32_t artwork_id = 0;   // refers to Artwork.id
    std::uint16_t orientation = 0;  // 0, 90, 180, 270

    bool operator==(const Placement& o) const {
        return loc==o.loc && artwork_id==o.artwork_id && orientation==o.orientation;
    }
};

struct PlacementHash {
    std::size_t operator()(const Placement& p) const {
        std::size_t seed = 0;
        hash_combine(seed, LocationHash{}(p.loc));
        hash_combine(seed, hash_u64(p.artwork_id));
        hash_combine(seed, hash_u64(p.orientation));
        return seed;
    }
};
