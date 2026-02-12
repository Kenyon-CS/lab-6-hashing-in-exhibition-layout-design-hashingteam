#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include "placement.hpp"

// A Layout is just a set of placements (order should NOT matter).
// To hash it, we canonicalize by sorting and then hashing the sequence.
struct Layout {
    std::vector<Placement> placements;

    // Canonical string form (simple + debuggable)
    std::string canonical_string() const {
        std::vector<Placement> v = placements;
        std::sort(v.begin(), v.end(), [](const Placement& a, const Placement& b){
            if (a.loc.room != b.loc.room) return a.loc.room < b.loc.room;
            if (a.loc.wall != b.loc.wall) return a.loc.wall < b.loc.wall;
            if (a.artwork_id != b.artwork_id) return a.artwork_id < b.artwork_id;
            return a.orientation < b.orientation;
        });
        std::ostringstream out;
        for (const auto& p : v) {
            out << p.loc.room << "," << p.loc.wall << ","
                << p.artwork_id << "," << p.orientation << ";";
        }
        return out.str();
    }
};
