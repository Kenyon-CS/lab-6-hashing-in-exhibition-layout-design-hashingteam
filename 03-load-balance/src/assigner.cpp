#include "assigner.hpp"
#include <algorithm>

template <class RoomFn>
static AssignmentResult assign_with_room_function(const std::vector<std::uint32_t>& artwork_ids,
                                                  std::uint32_t m_rooms,
                                                  int bucket_size_B,
                                                  RoomFn&& room_for) {
    AssignmentResult r;
    r.room_to_artworks.assign(m_rooms, {});
    r.load_factor = m_rooms == 0 ? 0.0
                                 : static_cast<double>(artwork_ids.size()) / static_cast<double>(m_rooms);

    for (auto id : artwork_ids) {
        std::uint32_t room = room_for(id, m_rooms);
        r.room_to_artworks[room].push_back(id);
    }

    r.max_load = 0;
    r.overflows = 0;
    for (auto& bucket : r.room_to_artworks) {
        int load = static_cast<int>(bucket.size());
        r.max_load = std::max(r.max_load, load);
        if (load > bucket_size_B) r.overflows++;
    }
    return r;
}

AssignmentResult HashedAssigner::assign(const std::vector<std::uint32_t>& artwork_ids,
                                       std::uint32_t m_rooms,
                                       int bucket_size_B) const {
    return assign_with_room_function(artwork_ids, m_rooms, bucket_size_B,
                                     [this](std::uint32_t id, std::uint32_t rooms) {
                                         return room_for(id, rooms);
                                     });
}

AssignmentResult BadHashAssigner::assign(const std::vector<std::uint32_t>& artwork_ids,
                                         std::uint32_t m_rooms,
                                         int bucket_size_B) const {
    return assign_with_room_function(artwork_ids, m_rooms, bucket_size_B,
                                     [this](std::uint32_t id, std::uint32_t rooms) {
                                         return room_for(id, rooms);
                                     });
}
