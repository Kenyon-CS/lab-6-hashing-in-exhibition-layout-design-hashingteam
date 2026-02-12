#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <type_traits>

// Simple hash-combine utility (similar idea to boost::hash_combine)
inline void hash_combine(std::size_t& seed, std::size_t v) {
    seed ^= v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
}

template <class T>
inline std::size_t hash_one(const T& x) {
    return std::hash<T>{}(x);
}

// Helper for hashing integral IDs consistently (avoid accidental char hashing)
inline std::size_t hash_u64(std::uint64_t x) {
    return std::hash<std::uint64_t>{}(x);
}
