# Group 1 — Detecting Duplicate Layouts: Write-Up

## Hash Function

We implemented a **polynomial rolling hash** over the canonical string representation of each layout. The string is treated as a sequence of base-131 digits evaluated modulo the Mersenne prime 2⁶¹ − 1:

```
h = c[0]·BASE^(n−1) + c[1]·BASE^(n−2) + … + c[n−1]   (mod 2^61 − 1)
```

**Why BASE = 131:** It exceeds 127 (the highest printable ASCII value), so each character in the canonical string maps to a unique "digit." This prevents accidental collisions between strings that differ only in how characters group together.

**Why MOD = 2⁶¹ − 1:** This is a Mersenne prime, which minimises collision probability — roughly `string_length / 2^61` per pair, effectively negligible for our 40-character canonical strings. Mersenne primes also allow fast modular reduction using bit operations. `__uint128_t` is used for the intermediate `h * BASE + c` product to prevent 64-bit overflow before reduction.

We also implemented `std::hash<string>` (the C++ built-in) as a reference baseline.

---

## Canonical Representation

Before hashing, each layout is **canonicalized**: placements are sorted lexicographically by `(room, wall, artwork, orientation)` and serialised as `"r,w,a,o;r,w,a,o;…"`. This ensures that two layouts containing the same placements in any order produce the identical byte sequence — the hash is then computed over this canonical string.

---

## Collision Handling

We use a `SeenSet` — an `unordered_map<size_t, vector<string>>` — that maps each hash value to a list of canonical strings that produced it. On every lookup:

1. Compute the hash → find the bucket in O(1).
2. Walk the bucket (usually 1 entry): compare canonical strings exactly.
3. A match on the full string → genuine duplicate. A hash match without a string match → hash collision, **not** a duplicate.

This makes the system **collision-safe**: no false positives are possible, even on hash collisions.

---

## Performance Results

### Small dataset (R1–R2, W1–W2, A1–A3)

| Layout | Input order | Canonical string | Result |
|--------|-------------|-----------------|--------|
| Layout 1 | `(R1,W1,A1,0) \| (R1,W2,A2,90) \| (R2,W1,A3,0)` | `"1,1,1,0;1,2,2,90;2,1,3,0;"` | New |
| Layout 2 | `(R2,W1,A3,0) \| (R1,W2,A2,90) \| (R1,W1,A1,0)` | `"1,1,1,0;1,2,2,90;2,1,3,0;"` | **DUPLICATE** |
| Layout 3 | `(R1,W1,A1,0) \| (R1,W2,A2,90) \| (R2,W1,A4,0)` | `"1,1,1,0;1,2,2,90;2,1,4,0;"` | New |
| Layout 4 | *(same as Layout 3)* | `"1,1,1,0;1,2,2,90;2,1,4,0;"` | **DUPLICATE** |

### Large dataset (R1–R5, W1–W6, A1–A20, 10,000 layouts: 8,000 unique + 2,000 injected duplicates)

| Method | Duplicates Found | Time |
|--------|-----------------|------|
| Brute-force O(n²) | 2,000 | 23.22 ms |
| Polynomial rolling hash | 2,000 | 4.16 ms |
| std::hash\<string\> | 2,000 | 0.80 ms |

All three methods agreed on the duplicate count, confirming correctness. The polynomial hash was **5.6× faster** than brute-force; `std::hash` was **29× faster** (the C++ runtime's hash is heavily optimised at the machine level, while our polynomial hash is portable and mathematically explicit).

---

## What Kinds of Duplicates Were Eliminated

The system eliminates **permutation duplicates**: layouts that contain exactly the same set of `(room, wall, artwork, orientation)` placements but were generated in different orders. This naturally arises when:

- A greedy algorithm considers placements in different traversal orders across runs.
- A backtracking search reaches the same configuration via different branching paths.
- A local-search step undoes and re-applies the same moves in a different sequence.

Without deduplication, each of these would be evaluated as a distinct candidate, wasting computation. The canonical sort + hash approach catches all such cases in O(k log k) time per layout (where k = number of placements), compared to O(n·k) for brute-force scanning.
