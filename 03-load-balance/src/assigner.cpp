#include "assigner.hpp"
#include <algorithm>

AssignmentResult HashedAssigner::assign(const std::vector<std::uint32_t>& artwork_ids,
                                       std::uint32_t m_rooms,
                                       int bucket_size_B) const {
    AssignmentResult r;
    r.room_to_artworks.assign(m_rooms, {});

    for (auto id : artwork_ids) {
        std::uint32_t room = room_for(id, m_rooms);
        r.room_to_artworks[room].push_back(id);
    }

    r.max_load = 0;
    r.overflows = 0;
    for (auto& bucket : r.room_to_artworks) {
        int load = (int)bucket.size();
        r.max_load = std::max(r.max_load, load);
        if (load > bucket_size_B) r.overflows++;
    }
    return r;
}
