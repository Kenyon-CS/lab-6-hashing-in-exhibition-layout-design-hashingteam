#include "constraints.hpp"

ConstraintChecker::ConstraintChecker(int max_sculptures_per_room,
                                     int max_same_medium_per_room)
    : max_sculptures(max_sculptures_per_room),
      max_same_medium(max_same_medium_per_room) {
    auto add_conflict_pair = [this](std::uint16_t left_wall, std::uint16_t right_wall) {
        for (std::uint16_t room = 1; room <= 32; ++room) {
            conflicting_walls[{room, left_wall}] = {room, right_wall};
            conflicting_walls[{room, right_wall}] = {room, left_wall};
        }
    };

    // Example gallery rule: paired sightline walls in the same room cannot both
    // be occupied simultaneously.
    add_conflict_pair(1, 2);
    add_conflict_pair(3, 4);
}

bool ConstraintChecker::is_allowed_wall_for_medium(const Placement& p,
                                                   const Artwork& art) const {
    if (art.medium == "sculpture") {
        // Treat even-numbered walls as floor/display spots for sculptures.
        return (p.loc.wall % 2) == 0;
    }
    return true;
}

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

    // 4) no more than K artworks of the same medium in a room
    RoomMediumKey medium_key{p.loc.room, art.medium};
    auto medium_it = medium_count_in_room.find(medium_key);
    int medium_count = (medium_it == medium_count_in_room.end()) ? 0 : medium_it->second;
    if (medium_count + 1 > max_same_medium) return false;

    // 5) some room-local wall pairs conflict and cannot both be active
    auto conflict_it = conflicting_walls.find(p.loc);
    if (conflict_it != conflicting_walls.end()
        && occupied.find(conflict_it->second) != occupied.end()) {
        return false;
    }

    // 6) sculptures may only be placed on designated display spots
    if (!is_allowed_wall_for_medium(p, art)) return false;

    return true;
}

void ConstraintChecker::place(const Placement& p, const Artwork& art) {
    occupied.insert(p.loc);
    room_artist_used.insert(RoomArtistKey{p.loc.room, art.artist});
    if (art.medium == "sculpture") {
        sculptures_in_room[p.loc.room] += 1;
    }
    medium_count_in_room[RoomMediumKey{p.loc.room, art.medium}] += 1;
}

void ConstraintChecker::unplace(const Placement& p, const Artwork& art) {
    occupied.erase(p.loc);
    room_artist_used.erase(RoomArtistKey{p.loc.room, art.artist});

    if (art.medium == "sculpture") {
        auto it = sculptures_in_room.find(p.loc.room);
        if (it != sculptures_in_room.end()) {
            if (--(it->second) == 0) sculptures_in_room.erase(it);
        }
    }

    RoomMediumKey medium_key{p.loc.room, art.medium};
    auto medium_it = medium_count_in_room.find(medium_key);
    if (medium_it != medium_count_in_room.end()) {
        if (--(medium_it->second) == 0) medium_count_in_room.erase(medium_it);
    }
}
