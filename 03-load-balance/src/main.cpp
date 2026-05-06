#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <iomanip>
#include <algorithm>

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

static void print_room_loads(const AssignmentResult& result) {
    for (std::size_t r = 0; r < result.room_to_artworks.size(); ++r) {
        std::cout << "Room " << r << " load=" << result.room_to_artworks[r].size() << " : ";
        int shown = 0;
        for (auto id : result.room_to_artworks[r]) {
            if (shown++ >= 12) {
                std::cout << "...";
                break;
            }
            std::cout << id << " ";
        }
        std::cout << "\n";
    }
}

static void print_summary(const AssignmentResult& result, int bucket_size_B) {
    std::cout << "alpha = n/m  = " << result.load_factor << "\n";
    std::cout << "bucket size B = " << bucket_size_B << "\n";
    std::cout << "max load = " << result.max_load << "\n";
    std::cout << "rooms overflowing (load > B) = " << result.overflows << "\n";
}

static void run_good_hash_trials(const std::vector<std::uint32_t>& ids,
                                 std::uint32_t m_rooms,
                                 int bucket_size_B) {
    struct Trial {
        std::uint64_t a, b;
    };

    const Trial trials[] = {
        {2654435761ULL, 12345ULL},
        {11400714819323198485ULL, 97ULL},
        {40503ULL, 7919ULL}
    };

    std::cout << "\nGood hash trials\n";
    for (const auto& t : trials) {
        HashedAssigner assigner(t.a, t.b, 4294967311ULL);
        auto result = assigner.assign(ids, m_rooms, bucket_size_B);
        std::cout << "A=" << t.a
                  << " B=" << t.b
                  << " -> max_load=" << result.max_load
                  << ", overflows=" << result.overflows << "\n";
    }
}

static void run_hash_comparison(const std::vector<std::uint32_t>& ids,
                                std::uint32_t m_rooms,
                                int bucket_size_B) {
    HashedAssigner good(2654435761ULL, 12345ULL, 4294967311ULL);
    BadHashAssigner bad(25);

    auto good_result = good.assign(ids, m_rooms, bucket_size_B);
    auto bad_result = bad.assign(ids, m_rooms, bucket_size_B);

    std::cout << "\nGood hash vs bad hash\n";
    std::cout << std::left << std::setw(12) << "Mode"
              << std::right << std::setw(12) << "max_load"
              << std::setw(14) << "overflows" << "\n";
    std::cout << std::left << std::setw(12) << "universal"
              << std::right << std::setw(12) << good_result.max_load
              << std::setw(14) << good_result.overflows << "\n";
    std::cout << std::left << std::setw(12) << "bad-span25"
              << std::right << std::setw(12) << bad_result.max_load
              << std::setw(14) << bad_result.overflows << "\n";
}

static void run_alpha_bucket_experiments(const std::vector<std::uint32_t>& ids) {
    HashedAssigner assigner(2654435761ULL, 12345ULL, 4294967311ULL);
    const std::uint32_t room_options[] = {5, 10, 20};
    const int bucket_options[] = {4, 6, 10};

    std::cout << "\nAlpha / bucket-size sweep\n";
    std::cout << std::left << std::setw(8) << "m"
              << std::setw(10) << "alpha"
