#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <chrono>

#include "constraints.hpp"

// Input files:
// 1) artworks file: id,artist,medium,title
// 2) placement attempts: room wall artwork orientation
//
// Usage:
//   ./02_conflict_check data/small_artworks.csv data/small_attempts.txt

static std::string trim(const std::string& s) {
    const std::size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const std::size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

static std::string strip_comment(const std::string& line) {
    return trim(line.substr(0, line.find('#')));
}

static bool load_artworks(const std::string& path, std::unordered_map<std::uint32_t, Artwork>& arts) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        line = strip_comment(line);
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string id_s, artist, medium, title;
        if (!std::getline(ss, id_s, ',')) return false;
        if (!std::getline(ss, artist, ',')) return false;
        if (!std::getline(ss, medium, ',')) return false;
        if (!std::getline(ss, title)) title = "";
        Artwork a;
        a.id = (std::uint32_t)std::stoul(id_s);
        a.artist = artist;
        a.medium = medium;
        a.title = title;
        arts[a.id] = a;
    }
    return true;
}

static bool load_attempts(const std::string& path, std::vector<Placement>& out) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        line = strip_comment(line);
        if (line.empty()) continue;
        Placement p;
        if (!(std::istringstream(line) >> p.loc.room >> p.loc.wall >> p.artwork_id >> p.orientation)) {
            std::cerr << "Bad line: " << line << "\n";
            return false;
        }
        out.push_back(p);
    }
    return true;
}

int main(int argc, char** argv) {
    std::string artFile = (argc >= 2) ? argv[1] : "data/small_artworks.csv";
    std::string attFile = (argc >= 3) ? argv[2] : "data/small_attempts.txt";
    auto start = std::chrono::steady_clock::now();

    std::unordered_map<std::uint32_t, Artwork> artworks;
    if (!load_artworks(artFile, artworks)) {
        std::cerr << "Failed to load artworks: " << artFile << "\n";
        return 1;
    }

    std::vector<Placement> attempts;
    if (!load_attempts(attFile, attempts)) {
        std::cerr << "Failed to load attempts: " << attFile << "\n";
        return 1;
    }

    ConstraintChecker checker(/*max_sculptures_per_room=*/2,
                              /*max_same_medium_per_room=*/2);

    int accepted = 0, rejected = 0;
    for (const auto& p : attempts) {
        auto it = artworks.find(p.artwork_id);
        if (it == artworks.end()) {
            std::cerr << "Unknown artwork id " << p.artwork_id << "\n";
            continue;
        }
        const Artwork& art = it->second;

        if (checker.can_place(p, art)) {
            checker.place(p, art);
            accepted++;
            std::cout << "ACCEPT  room " << p.loc.room << " wall " << p.loc.wall
