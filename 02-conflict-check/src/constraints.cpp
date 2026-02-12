#include "constraints.hpp"

bool ConstraintChecker::can_place(const Placement& p, const Artwork& art) const {
    // 1) location is free
    if (occupied.find(p.loc) != occupied.end()) return false;

    // 2) no duplicate artist in same room
    RoomArtistKey k{p.loc.room, art.artist};
    if (room_artist_used.find(k) != room_artist_used.end()) return false;

    // 3) sculptures cap
    if (art.medium == "sculpture") {
        auto it = sculptures_in_room.find(p.loc.room);
        int cur = (it == sculptures_in_room.end()) ? 0 : it->second;
        if (cur + 1 > max_sculptures) return false;
    }

    return true;
}

void ConstraintChecker::place(const Placement& p, const Artwork& art) {
    occupied.insert(p.loc);
    room_artist_used.insert(RoomArtistKey{p.loc.room, art.artist});
    if (art.medium == "sculpture") {
        sculptures_in_room[p.loc.room] += 1;
    }
}
