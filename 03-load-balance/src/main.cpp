#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

#include "assigner.hpp"

// Usage:
//   ./03_load_balance data/small_artworks.txt 3 2
//   ./03_load_balance data/large_artworks.txt 10 6
//
// args:
//   file  m_rooms  bucket_size_B

static bool load_artwork_ids(const std::string& path, std::vector<std::uint32_t>& ids) {
    std::ifstream in(path);
    if (!in) return false;
    std::uint32_t x;
    while (in >> x) ids.push_back(x);
    return true;
}

int main(int argc, char** argv) {
    std::string file = (argc >= 2) ? argv[1] : "data/small_artworks.txt";
    std::uint32_t m_rooms = (argc >= 3) ? (std::uint32_t)std::stoul(argv[2]) : 3;
    int B = (argc >= 4) ? std::atoi(argv[3]) : 2;

    std::vector<std::uint32_t> ids;
    if (!load_artwork_ids(file, ids)) {
        std::cerr << "Failed to read " << file << "\n";
        return 1;
    }

    // A simple universal-hashing style setup (constants can be changed).
    // P should be a large prime > max artwork id.
    HashedAssigner assigner(/*a=*/2654435761ULL, /*b=*/12345ULL, /*prime=*/4294967311ULL);

    auto result = assigner.assign(ids, m_rooms, B);

    std::cout << "n (artworks) = " << ids.size() << "\n";
    std::cout << "m (rooms)    = " << m_rooms << "\n";
    std::cout << "alpha = n/m  = " << (double)ids.size() / (double)m_rooms << "\n";
    std::cout << "bucket size B = " << B << "\n\n";

    for (std::size_t r = 0; r < result.room_to_artworks.size(); r++) {
        std::cout << "Room " << r << " load=" << result.room_to_artworks[r].size() << " : ";
        // Print at most first 12 ids to keep output readable
        int shown = 0;
        for (auto id : result.room_to_artworks[r]) {
            if (shown++ >= 12) { std::cout << "..."; break; }
            std::cout << id << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\nmax load = " << result.max_load << "\n";
    std::cout << "rooms overflowing (load > B) = " << result.overflows << "\n";

    std::cout << "\nTODO (students):\n"
              << " - Try multiple (a,b) pairs; compare max load / overflow count.\n"
              << " - Compare with a non-random/poor hash (e.g., room = id % m).\n"
              << " - Run experiments for different alpha = n/m and bucket size B.\n";
    return 0;
}
