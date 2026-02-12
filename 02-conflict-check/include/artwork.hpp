#pragma once
#include <string>
#include <cstdint>

struct Artwork {
    // Keep IDs numeric for fast hashing; store human-readable labels separately.
    std::uint32_t id = 0;      // e.g., 1 for A1
    std::string   title;       // optional
    std::string   artist;      // used in Project 2 constraints
    std::string   medium;      // "painting", "sculpture", etc.

    Artwork() = default;
    Artwork(std::uint32_t id_, std::string t, std::string a, std::string m)
        : id(id_), title(std::move(t)), artist(std::move(a)), medium(std::move(m)) {}
};
