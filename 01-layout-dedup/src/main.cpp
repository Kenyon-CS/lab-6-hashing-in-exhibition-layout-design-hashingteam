#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>
#include <cstdint>

#include "layout.hpp"

// ─────────────────────────────────────────────────────────────────────────────
// Display helper — formats placements as "(R1,W1,A1,0deg) | ..."
// sorted=false keeps insertion order; sorted=true uses canonical order.
// Not part of the hashing pipeline — used only for readable console output.
// ─────────────────────────────────────────────────────────────────────────────
static std::string formatPlacements(const Layout& L, bool sorted) {
    std::vector<Placement> v = L.placements;
    if (sorted) {
        std::sort(v.begin(), v.end(), [](const Placement& a, const Placement& b) {
            if (a.loc.room   != b.loc.room)   return a.loc.room   < b.loc.room;
            if (a.loc.wall   != b.loc.wall)   return a.loc.wall   < b.loc.wall;
            if (a.artwork_id != b.artwork_id) return a.artwork_id < b.artwork_id;
            return a.orientation < b.orientation;
        });
    }
    std::ostringstream out;
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (i) out << " | ";
        out << "(R" << v[i].loc.room
            << ",W" << v[i].loc.wall
            << ",A" << v[i].artwork_id
            << "," << v[i].orientation << "deg)";
    }
    return out.str();
}

// =============================================================================
// FUNCTION: canonicalize(layout) -> string
//
// Produces an order-independent representation of a layout by:
//   1. Sorting placements lexicographically: (room, wall, artwork, orientation)
//   2. Serialising each placement as "r,w,a,o;" and concatenating.
//
// Key property: any permutation of the same placement set yields the exact same
// byte sequence. Two layouts are identical iff their canonical strings match.
//
// Example: { (R2,W1,A3,0), (R1,W1,A1,0) } -> "1,1,1,0;2,1,3,0;"
// =============================================================================
std::string canonicalize(const Layout& layout) {
    // Delegates to Layout::canonical_string() defined in layout.hpp,
    // which performs the sort + serialize described above.
    return layout.canonical_string();
}

// =============================================================================
// FUNCTION: hashLayout(canonical) -> size_t
//
// Two implementations are provided so their speed and collision properties
// can be compared on the large dataset.
//
// --- Implementation A: Polynomial Rolling Hash ---
//
// Models the string as a degree-n polynomial evaluated at BASE (mod MOD):
//   h = c[0]*BASE^(n-1) + c[1]*BASE^(n-2) + ... + c[n-1]   (mod MOD)
//
// Parameter choices:
//   BASE = 131: larger than 127 (max printable ASCII), so each character maps
//               to a unique "digit" — prevents aliasing between multi-char runs.
//   MOD  = 2^61 - 1: a Mersenne prime. Large enough that collision probability
//               per pair ≈ len/2^61, effectively negligible for our strings.
//               Mersenne-prime reduction is also fast (bit tricks).
//   __uint128_t: prevents 64-bit overflow in the intermediate (h * BASE + c)
//               before the modulo reduction is applied.
//
// --- Implementation B: std::hash<string> ---
//
// The C++ standard-library hash. Implementation-defined (GCC/Clang typically
// use MurmurHash or SipHash-1-3). Fast and well-distributed; serves as a
// reference baseline to verify our polynomial hash is competitive.
// =============================================================================

static std::size_t polyRollingHash(const std::string& s) {
    constexpr std::uint64_t BASE = 131;
    constexpr std::uint64_t MOD  = (1ULL << 61) - 1;   // Mersenne prime
    std::uint64_t h = 0;
    for (unsigned char c : s) {
        // Cast to __uint128_t before multiply to avoid overflow before reduction
        h = static_cast<std::uint64_t>(
                (static_cast<__uint128_t>(h) * BASE + c) % MOD);
    }
    return static_cast<std::size_t>(h);
}

static std::size_t stdStringHash(const std::string& s) {
    return std::hash<std::string>{}(s);
}

enum class HashMethod { POLY, STD };

std::size_t hashLayout(const std::string& canonical,
                       HashMethod method = HashMethod::POLY) {
    return (method == HashMethod::POLY) ? polyRollingHash(canonical)
                                        : stdStringHash(canonical);
}

// =============================================================================
// Seen-Set Design
//
//   Type:  unordered_map<size_t, vector<string>>
//   Key:   hash value (computed by our chosen hash function)
//   Value: list of canonical strings that produced that hash
//
// Collision handling: if two DIFFERENT layouts happen to hash to the same
// value (a hash collision), they share a bucket (the vector). The exact
// string equality check inside isDuplicate prevents false positives —
// only a genuine canonical-string match is reported as a duplicate.
//
// This is strictly safer than using the hash value alone as the equality
// criterion, which would give wrong answers on collisions.
// =============================================================================
using SeenSet = std::unordered_map<std::size_t, std::vector<std::string>>;

// =============================================================================
// FUNCTION: isDuplicate(layout, seen) -> bool
//
// Steps:
//   1. canonicalize(layout)  — order-independent string
//   2. hashLayout(canonical) — O(len) hash computation
//   3. seen[hash]            — O(1) average bucket lookup
//   4. Walk bucket: if any stored string == canonical -> true (genuine dup)
//   5. Otherwise: push canonical into bucket, return false (new layout)
//
// Average complexity: O(k) where k = number of placements (dominates step 1).
// Worst case per call: O(k + b) where b = bucket size (usually 1; collisions
// are astronomically rare with our 61-bit Mersenne hash over 40-char strings).
// =============================================================================
bool isDuplicate(const Layout& layout, SeenSet& seen,
                 HashMethod method = HashMethod::POLY) {
    const std::string canon = canonicalize(layout);
    const std::size_t h     = hashLayout(canon, method);
    auto& bucket = seen[h];
    for (const std::string& stored : bucket) {
        if (stored == canon) return true;       // genuine duplicate
    }
    bucket.push_back(canon);                    // first occurrence — record it
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// File parser: reads layouts stored as "room wall artwork orient" lines,
// one placement per line, blocks separated by blank lines.
// ─────────────────────────────────────────────────────────────────────────────
static bool parseLayouts(const std::string& path, std::vector<Layout>& out) {
    std::ifstream in(path);
    if (!in) return false;
    Layout cur;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) {
            if (!cur.placements.empty()) {
                out.push_back(std::move(cur));
                cur.placements.clear();
            }
            continue;
        }
        Placement p;
        std::istringstream ss(line);
        if (!(ss >> p.loc.room >> p.loc.wall >> p.artwork_id >> p.orientation)) {
            std::cerr << "Bad line: " << line << "\n";
            return false;
        }
        cur.placements.push_back(p);
    }
    if (!cur.placements.empty()) out.push_back(std::move(cur));
    return true;
}

// =============================================================================
// FUNCTION: runSmallTest()
//
// Dataset: Rooms R1-R2, Walls W1-W2, Artworks A1-A3, orientations {0,90,180,270}
//
// Shows for each layout:
//   - Placements in the order they were given (to prove order doesn't matter)
//   - The canonical (sorted) form
//   - The raw canonical string
//   - Both hash values (polynomial and std::hash)
//   - Whether it was detected as a duplicate
//
// Expected: Layout 2 is a shuffle of Layout 1 → must be flagged as duplicate.
//           Layout 4 is an exact copy of Layout 3 → also flagged.
// =============================================================================
void runSmallTest() {
    std::cout
        << "============================================================\n"
        << "  SMALL DATASET TEST\n"
        << "  Rooms: R1,R2  |  Walls: W1,W2  |  Artworks: A1-A3\n"
        << "  Orientations: 0, 90, 180, 270\n"
        << "============================================================\n\n";

    // Aggregate-initialise: { {room,wall}, artwork_id, orientation }
    Layout L1, L2, L3, L4;
    L1.placements = { {{1,1}, 1,   0}, {{1,2}, 2,  90}, {{2,1}, 3,  0} };
    L2.placements = { {{2,1}, 3,   0}, {{1,2}, 2,  90}, {{1,1}, 1,  0} };  // same as L1
    L3.placements = { {{1,1}, 1,   0}, {{1,2}, 2,  90}, {{2,1}, 4,  0} };  // A4 not A3
    L4.placements = { {{1,1}, 1,   0}, {{1,2}, 2,  90}, {{2,1}, 4,  0} };  // copy of L3

    const struct { const Layout* L; const char* label; } cases[] = {
        { &L1, "Layout 1 — base layout" },
        { &L2, "Layout 2 — same as Layout 1, shuffled order" },
        { &L3, "Layout 3 — different artwork (A4 instead of A3)" },
        { &L4, "Layout 4 — duplicate of Layout 3" },
    };

    SeenSet seen;
    for (const auto& c : cases) {
        const std::string canon = canonicalize(*c.L);
        const std::size_t polyH = polyRollingHash(canon);
        const std::size_t stdH  = stdStringHash(canon);
        const bool        dup   = isDuplicate(*c.L, seen);

        std::cout << "── " << c.label << " ──\n";
        std::cout << "  Input (as given)  : " << formatPlacements(*c.L, false) << "\n";
        std::cout << "  Canonical (sorted) : " << formatPlacements(*c.L, true)  << "\n";
        std::cout << "  Canonical string   : \"" << canon << "\"\n";
        std::cout << "  Poly rolling hash  : " << polyH << "\n";
        std::cout << "  std::hash<string>  : " << stdH  << "\n";
        std::cout << "  Result             : "
                  << (dup ? "*** DUPLICATE DETECTED ***"
                          : "New layout — added to seen set") << "\n\n";
    }

    // Also verify against the data file (should match above observations)
    std::vector<Layout> file_layouts;
    if (parseLayouts("data/small_layouts.txt", file_layouts)) {
        std::cout << "── Verification from data/small_layouts.txt ──\n";
        SeenSet seen_file;
        int dups = 0;
        for (std::size_t i = 0; i < file_layouts.size(); ++i) {
            bool d = isDuplicate(file_layouts[i], seen_file);
            std::cout << "  Layout " << (i + 1) << ": "
                      << (d ? "DUPLICATE" : "unique") << "\n";
            if (d) ++dups;
        }
        std::cout << "  Summary: " << file_layouts.size() << " layouts, "
                  << dups << " duplicate(s) detected\n";
    }
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// Random layout generator for the large test.
// Space: R1-R5, W1-W6, A1-A20, {0, 90, 180, 270}.
// Picks `size` distinct (room,wall) slots (no wall assigned twice per layout),
// then assigns a random artwork and orientation to each slot.
// ─────────────────────────────────────────────────────────────────────────────
static Layout generateRandom(std::mt19937& rng, int size) {
    constexpr int ROOMS = 5, WALLS = 6, ARTWORKS = 20;
    constexpr int ORI[4] = {0, 90, 180, 270};

    // Build all 30 (room,wall) pairs, shuffle, take first `size`
    std::vector<std::pair<int,int>> slots;
    slots.reserve(ROOMS * WALLS);
    for (int r = 1; r <= ROOMS; ++r)
        for (int w = 1; w <= WALLS; ++w)
            slots.push_back({r, w});
    std::shuffle(slots.begin(), slots.end(), rng);

    std::uniform_int_distribution<int> artDist(1, ARTWORKS);
    std::uniform_int_distribution<int> oriDist(0, 3);

    Layout L;
    L.placements.reserve(size);
    for (int i = 0; i < size; ++i) {
        Placement p;
        p.loc.room    = static_cast<std::uint16_t>(slots[i].first);
        p.loc.wall    = static_cast<std::uint16_t>(slots[i].second);
        p.artwork_id  = static_cast<std::uint32_t>(artDist(rng));
        p.orientation = static_cast<std::uint16_t>(ORI[oriDist(rng)]);
        L.placements.push_back(p);
    }
    return L;
}

// =============================================================================
// FUNCTION: runLargeTest()
//
// Generates 10,000 candidate layouts: 8,000 unique + 2,000 exact copies
// injected at random positions (so duplicates are scattered throughout).
//
// Compares three duplicate-detection strategies:
//
//   1. Brute-force O(n²): for each layout, scan all previously seen unique
//      canonical strings for a match. O(n * u) where u = unique count so far.
//
//   2. Polynomial rolling hash: hash(canonical) -> bucket -> exact string check.
//      O(n) average, O(n * b) worst case (b = max bucket size, usually 1).
//
//   3. std::hash<string>: identical structure to method 2 but uses the C++
//      built-in hash function. Comparison baseline for method 2.
//
// Timing note: canonical strings are pre-computed once and reused by all three
// methods, so we measure ONLY the duplicate-detection step (lookup + comparison)
// and not the canonicalization cost, which is identical for all methods.
// =============================================================================
void runLargeTest() {
    std::cout
        << "============================================================\n"
        << "  LARGE DATASET TEST  (10,000 layouts)\n"
        << "  Rooms: R1-R5  |  Walls: W1-W6  |  Artworks: A1-A20\n"
        << "  Orientations: 0, 90, 180, 270\n"
        << "============================================================\n\n";

    constexpr int TOTAL          = 10'000;
    constexpr int UNIQUE_BASE    = 8'000;   // genuinely distinct layouts
    constexpr int PLACEMENT_SIZE = 8;       // placements per layout

    std::mt19937 rng(42);   // fixed seed → reproducible output

    // Build 8,000 random unique base layouts
    std::vector<Layout> layouts;
    layouts.reserve(TOTAL);
    for (int i = 0; i < UNIQUE_BASE; ++i)
        layouts.push_back(generateRandom(rng, PLACEMENT_SIZE));

    // Inject 2,000 exact duplicates at random positions
    std::uniform_int_distribution<int> pick(0, UNIQUE_BASE - 1);
    for (int i = 0; i < TOTAL - UNIQUE_BASE; ++i)
        layouts.push_back(layouts[pick(rng)]);

    // Shuffle so duplicates are interleaved uniformly throughout the list
    std::shuffle(layouts.begin(), layouts.end(), rng);

    std::cout << "Total layouts generated : " << TOTAL       << "\n";
    std::cout << "Unique (expected)       : " << UNIQUE_BASE << "\n";
    std::cout << "Duplicates (injected)   : " << (TOTAL - UNIQUE_BASE) << "\n\n";

    // Pre-compute canonical strings once — all three methods share this work
    // so the timing comparison reflects only lookup / comparison cost.
    std::vector<std::string> canonicals(TOTAL);
    for (int i = 0; i < TOTAL; ++i)
        canonicals[i] = canonicalize(layouts[i]);

    // ── Method 1: Brute-force O(n²) ──────────────────────────────────────────
    // For each layout, linearly scan every canonical string seen so far.
    // No hash involved — pure string equality, O(n * u) total comparisons.
    long long bf_dups = 0;
    double    bf_ms   = 0.0;
    {
        auto t0 = std::chrono::high_resolution_clock::now();

        std::vector<std::string> unique_seen;
        unique_seen.reserve(UNIQUE_BASE);
        for (int i = 0; i < TOTAL; ++i) {
            bool found = false;
            for (const auto& prev : unique_seen) {
                if (prev == canonicals[i]) { found = true; break; }
            }
            if (found) ++bf_dups;
            else       unique_seen.push_back(canonicals[i]);
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        bf_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // ── Method 2: Polynomial Rolling Hash ────────────────────────────────────
    // hash(canonical) -> SeenSet bucket -> exact string equality check.
    // O(n) average; each lookup touches only the strings in one hash bucket.
    long long poly_dups = 0;
    double    poly_ms   = 0.0;
    {
        auto t0 = std::chrono::high_resolution_clock::now();

        SeenSet seen;
        for (int i = 0; i < TOTAL; ++i) {
            const std::size_t h = polyRollingHash(canonicals[i]);
            auto& bucket = seen[h];
            bool found = false;
            for (const auto& s : bucket) {
                if (s == canonicals[i]) { found = true; break; }
            }
            if (found) ++poly_dups;
            else       bucket.push_back(canonicals[i]);
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        poly_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // ── Method 3: std::hash<string> ──────────────────────────────────────────
    // Same SeenSet structure as method 2 but uses the C++ built-in hash.
    // Lets us compare our polynomial hash against the standard implementation.
    long long std_dups = 0;
    double    std_ms   = 0.0;
    {
        auto t0 = std::chrono::high_resolution_clock::now();

        SeenSet seen;
        for (int i = 0; i < TOTAL; ++i) {
            const std::size_t h = stdStringHash(canonicals[i]);
            auto& bucket = seen[h];
            bool found = false;
            for (const auto& s : bucket) {
                if (s == canonicals[i]) { found = true; break; }
            }
            if (found) ++std_dups;
            else       bucket.push_back(canonicals[i]);
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        std_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // ── Results table ─────────────────────────────────────────────────────────
    const int W1 = 30, W2 = 20, W3 = 12;
    std::cout << std::left
              << std::setw(W1) << "Method"
              << std::setw(W2) << "Duplicates Found"
              << std::setw(W3) << "Time (ms)" << "\n"
              << std::string(W1 + W2 + W3, '-') << "\n";

    auto row = [&](const std::string& name, long long dups, double ms) {
        std::cout << std::left
                  << std::setw(W1) << name
                  << std::setw(W2) << dups
                  << std::fixed << std::setprecision(2) << ms << "\n";
    };
    row("Brute-force O(n^2)",       bf_dups,   bf_ms);
    row("Polynomial Rolling Hash",  poly_dups, poly_ms);
    row("std::hash<string>",        std_dups,  std_ms);
    std::cout << std::string(W1 + W2 + W3, '-') << "\n\n";

    // Speed-up ratios relative to brute force
    std::cout << "Speed-up vs brute-force O(n^2):\n";
    if (poly_ms > 0.0)
        std::cout << "  Polynomial hash : "
                  << std::fixed << std::setprecision(1)
                  << (bf_ms / poly_ms) << "x faster\n";
    if (std_ms > 0.0)
        std::cout << "  std::hash       : "
                  << std::fixed << std::setprecision(1)
                  << (bf_ms / std_ms)  << "x faster\n";
    std::cout << "\n";

    // Correctness check: all three methods must agree
    if (bf_dups == poly_dups && poly_dups == std_dups) {
        std::cout << "Correctness: all three methods agree ("
                  << bf_dups << " duplicates found).\n";
    } else {
        std::cout << "WARNING: methods disagree!\n"
                  << "  Brute-force=" << bf_dups
                  << "  Poly="        << poly_dups
                  << "  Std="         << std_dups << "\n";
    }
}

int main() {
    runSmallTest();
    runLargeTest();
    return 0;
}
