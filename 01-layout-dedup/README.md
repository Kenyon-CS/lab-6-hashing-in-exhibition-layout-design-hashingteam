# Project 1: Layout Deduplication with Hashing

## Goal
When you generate candidate exhibition layouts (greedy, backtracking, local search),
you must avoid evaluating the *same* layout repeatedly. This project builds the
hash-based framework for **state deduplication**.

A layout is a set of placements:
- placement = (room_id, wall_id, artwork_id, orientation)

Two layouts are considered identical if they contain the same placements,
even if the placements appear in a different order.

## What’s Provided
- `Artwork`, `Location`, `Placement` structs (with hash helpers)
- `Layout` struct with a **canonical string** representation
- `main.cpp` that:
  1. reads layouts from a file
  2. canonicalizes each layout
  3. stores it in an `unordered_set`
  4. reports duplicates

This is intentionally simple. Your job is to improve it.

## Build & Run
From this directory:

    make
    ./01_layout_dedup data/small_layouts.txt
    ./01_layout_dedup data/large_layouts.txt

Or:

    make run

## Input Format
Layouts are separated by a blank line.

Each placement line is:

    room wall artwork orientation

Example:

    1 1 1 0
    1 2 2 90
    2 1 3 0

(blank line)
(next layout...)

## Tasks (What Students Add)
### A. Better Hashing (faster than strings)
Right now, dedup uses `canonical_string()` as the key. This is robust but slow.

Replace/augment with:
- A faster numeric hash of the sorted placements
- Or **Zobrist hashing**:
  - Pre-generate random 64-bit values for each (room, wall, artwork, orientation)
  - Layout hash = XOR of the placements' random values
  - Supports O(1) incremental updates during search

Collision handling idea:
- Store both:
  - the 64-bit hash
  - the canonical string (or sorted placement list)
- Use the hash to index quickly, and verify equality on collisions.

### B. Symmetry Reduction (optional extension)
If two layouts are equivalent under symmetry, treat them as the same.
Examples:
- swapping two identical rooms
- mirroring a rectangular room
- rotating by 180 degrees

Approach:
- define a `normalize(layout)` function that transforms to a canonical symmetry form
- then hash the normalized form

### C. Measure the Impact
On `data/large_layouts.txt`:
- count duplicates found
- time the run before/after optimization

## Deliverable
A short write-up:
- your chosen hash function
- collision strategy
- performance results (small + large)
- what kinds of duplicate layouts you eliminated

