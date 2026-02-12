#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "constraints.hpp"

// Input files:
// 1) artworks file: id,artist,medium,title
// 2) placement attempts: room wall artwork orientation
//
// Usage:
//   ./02_conflict_check data/small_artworks.csv data/small_attempts.txt

static bool load_artworks(const std::string& path, std::unordered_map<std::uint32_t, Artwork>& arts) {
    std::ifstream in(path);
    if (!in) return false;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        if (line.rfind("#", 0) == 0) continue;
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
        if (line.empty()) continue;
        if (line.rfind("#", 0) == 0) continue;
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

    ConstraintChecker checker(/*max_sculptures_per_room=*/2);

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
                      << " art " << p.artwork_id << " (" << art.artist << ", " << art.medium << ")\n";
        } else {
            rejected++;
            std::cout << "REJECT  room " << p.loc.room << " wall " << p.loc.wall
                      << " art " << p.artwork_id << " (" << art.artist << ", " << art.medium << ")\n";
        }
    }

    std::cout << "\nAccepted: " << accepted << "\nRejected: " << rejected << "\n";
    std::cout << "\nTODO (students):\n"
              << " - Add more constraints (lighting, adjacency, wall capacity, etc.).\n"
              << " - Support 'remove' for backtracking search.\n"
              << " - Stress test runtime as attempts scale.\n";
    return 0;
}
