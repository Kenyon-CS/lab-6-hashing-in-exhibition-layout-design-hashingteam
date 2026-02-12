#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <string>

#include "layout.hpp"

// Input format (simple):
// Each layout is separated by a blank line.
// Each placement line:  room wall artwork orientation
// Example:
// 1 1 1 0
// 1 2 2 90
// 2 1 3 0
//
// <blank line>
// ...

static bool parse_layouts(const std::string& path, std::vector<Layout>& out) {
    std::ifstream in(path);
    if (!in) return false;

    Layout cur;
    std::string line;
    while (std::getline(in, line)) {
        if (line.size() == 0) {
            if (!cur.placements.empty()) {
                out.push_back(cur);
                cur.placements.clear();
            }
            continue;
        }
        Placement p;
        if (!(std::istringstream(line) >> p.loc.room >> p.loc.wall >> p.artwork_id >> p.orientation)) {
            std::cerr << "Bad line: " << line << "\n";
            return false;
        }
        cur.placements.push_back(p);
    }
    if (!cur.placements.empty()) out.push_back(cur);
    return true;
}

int main(int argc, char** argv) {
    std::string file = (argc >= 2) ? argv[1] : "data/small_layouts.txt";

    std::vector<Layout> layouts;
    if (!parse_layouts(file, layouts)) {
        std::cerr << "Failed to read " << file << "\n";
        return 1;
    }

    // Hash set of canonical strings (simple, robust collision handling: strings compare exactly).
    std::unordered_set<std::string> seen;

    int duplicates = 0;
    for (std::size_t i = 0; i < layouts.size(); i++) {
        const std::string key = layouts[i].canonical_string();
        if (seen.find(key) != seen.end()) {
            duplicates++;
            std::cout << "DUPLICATE layout at index " << i << "\n";
        } else {
            seen.insert(key);
        }
    }

    std::cout << "Layouts read: " << layouts.size() << "\n";
    std::cout << "Unique layouts: " << seen.size() << "\n";
    std::cout << "Duplicates: " << duplicates << "\n";

    std::cout << "\nTODO (students):\n"
              << " - Replace canonical_string hashing with a faster incremental hash (e.g., Zobrist/XOR).\n"
              << " - Add symmetry-reduction (e.g., room rotations/mirroring) before hashing.\n";
    return 0;
}
