# Software Rasteriser Optimisation

![C++20](https://img.shields.io/badge/C++20-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![AVX2](https://img.shields.io/badge/AVX2-FF6F00?style=for-the-badge&logoColor=white)
![SIMD](https://img.shields.io/badge/SIMD-FF6F00?style=for-the-badge&logoColor=white)

A CPU software rasteriser optimised for performance, reaching **3 to 5x speedups** through SIMD vectorisation, multithreading, and cache-aware restructuring. Written in C++20.

## Performance

| Scene 1 | Scene 2 | Scene 3 |
|:---:|:---:|:---:|
| ![](docs/Scene1Bar.png) | ![](docs/Scene2Bar.png) | ![](docs/Scene3Bar.png) |

## Output

| Scene 1 | Scene 2 | Scene 3 |
|:---:|:---:|:---:|
| ![](docs/Scene1Gif.gif) | ![](docs/Scene2Gif.gif) | ![](docs/Scene3Gif.gif) |

## Results

| Scene | Baseline (ms) | Optimised (ms) | Speedup |
|---|---|---|---|
| Scene 1 | 5364 | 1707 | 3.14x |
| Scene 2 | 2091 | 399 | 5.24x |
| Scene 3 | 5020 | 1472 | 3.41x |

## Approach

- **Profiled first** to find the hotspots, then optimised them in priority order rather than guessing.
- **AVX2 SIMD** vectorisation of the per-pixel and per-vertex hot paths.
- **Multithreading** with `std::jthread` across screen regions.
- **Cache-friendly** data restructuring, branch early-exits, and unrolled matrix math.

## Tech stack

C++20 · AVX2 · std::jthread

## Full technical write-up

The full breakdown of each optimisation and its measured impact is in
[docs/rasteriser_TECHNICAL.md](docs/rasteriser_TECHNICAL.md).


