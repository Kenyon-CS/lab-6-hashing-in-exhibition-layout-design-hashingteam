#pragma once
#include <cstdint>
#include <vector>
#include <random>
#include <string>
#include <unordered_map>
#include "hash_utils.hpp"

// Hash-based randomized assignment: room = h(artwork_id) mod m
// This is the "balls into bins" model as code.

struct AssignmentResult {
    std::vector<std::vector<std::uint32_t>> room_to_artworks; // chaining model
    int max_load = 0;
    int overflows = 0;
};

class HashedAssigner {
public:
    HashedAssigner(std::uint64_t a, std::uint64_t b, std::uint64_t prime)
        : A(a), B(b), P(prime) {}

    // Universal-hash style: h(x) = (A*x + B) mod P
    // then room = h(x) mod m
    std::uint32_t room_for(std::uint32_t artwork_id, std::uint32_t m) const {
        std::uint64_t x = artwork_id;
        std::uint64_t hx = (A * x + B) % P;
        return (std::uint32_t)(hx % m);
    }

    AssignmentResult assign(const std::vector<std::uint32_t>& artwork_ids,
                            std::uint32_t m_rooms,
                            int bucket_size_B) const;

private:
    std::uint64_t A, B, P;
};
