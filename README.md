# Software Rasteriser Optimisation

![C++20](https://img.shields.io/badge/C++20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![AVX2](https://img.shields.io/badge/AVX2-FF6F00?style=for-the-badge&logoColor=white)
![SIMD](https://img.shields.io/badge/SIMD-FF6F00?style=for-the-badge&logoColor=white)

A CPU software rasteriser optimised for performance, reaching **3 to 5x speedups** through SIMD vectorisation, multithreading, and cache-aware restructuring. Written in C++20.

![Output](docs/banner.png)
<!-- TODO: a rendered output screenshot, plus a before/after benchmark bar chart. The chart is your strongest visual for a performance project. -->

## Results

| Scene | Baseline (ms) | Optimised (ms) | Speedup |
|---|---|---|---|
| Scene A | TODO | TODO | 3.1x |
| Scene B | TODO | TODO | 5.2x |
| Scene C | TODO | TODO | 3.4x |

<!-- TODO: fill in the real timings from your report -->

## Approach

- **Profiled first** to find the hotspots, then optimised them in priority order rather than guessing.
- **AVX2 SIMD** vectorisation of the per-pixel and per-vertex hot paths.
- **Multithreading** with `std::jthread` across screen regions.
- **Cache-friendly** data restructuring, branch early-exits, and unrolled matrix math.

## Tech stack

C++20 · AVX2 · std::jthread

## Build & run

<!-- TODO: your build and benchmark steps -->
```
Requires: a C++20 compiler with AVX2 support.
git clone https://github.com/MeadowsJoe/<repo>.git
# build (Release / -O2 with AVX2 enabled), then run the benchmark
```

## Full technical write-up

The full breakdown of each optimisation and its measured impact is in
[docs/technical-writeup.pdf](docs/technical-writeup.pdf).
<!-- TODO: add your cleaned report here -->

## Future work

- AVX-512 path where available.
- GPU comparison baseline.
- Tile-based threading to cut false sharing.
