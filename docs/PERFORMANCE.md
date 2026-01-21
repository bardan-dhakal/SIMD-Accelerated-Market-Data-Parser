# Performance Analysis and Tuning Guide

This document provides detailed performance analysis, benchmark results, and tuning recommendations for the SIMD-Accelerated Market Data Parser.

## Table of Contents

1. [Performance Summary](#performance-summary)
2. [Benchmark Results](#benchmark-results)
3. [Performance Breakdown](#performance-breakdown)
4. [Hardware Requirements](#hardware-requirements)
5. [Tuning Guide](#tuning-guide)
6. [Profiling Methodology](#profiling-methodology)
7. [Bottleneck Analysis](#bottleneck-analysis)

---

## Performance Summary

### Key Metrics

| Metric | Scalar | SIMD (AVX-512) | Improvement |
|--------|--------|----------------|-------------|
| Delimiter Finding | ~80-100 ns | ~10-15 ns | **8-10x** |
| Full Message Parse | ~400-500 ns | ~60-80 ns | **6-8x** |
| Throughput | 2-2.5M msg/sec | 12-16M msg/sec | **6-8x** |
| Memory Bandwidth | ~200 MB/s | ~1.5 GB/s | **7.5x** |

### Latency Distribution (Medium Message ~90 bytes)

```
Percentile    Scalar        SIMD
─────────────────────────────────────
p50           420 ns        58 ns
p90           480 ns        72 ns
p99           550 ns        95 ns
p99.9         680 ns        140 ns
```

---

## Benchmark Results

### Test Environment

```
CPU:        Intel Core i9-12900K (Alder Lake)
            AVX-512 Support: Yes
Cores:      8P + 8E (16 total)
RAM:        64 GB DDR5-4800
OS:         Ubuntu 22.04 LTS
Compiler:   GCC 11.4.0
Flags:      -O3 -march=native -mavx512f -mavx512bw
```

### Delimiter Finding Benchmarks

```
Benchmark                              Time        CPU     Iterations
─────────────────────────────────────────────────────────────────────
BM_Find_Delimiters_Scalar              85 ns      84 ns      8341259
BM_Find_Delimiters_SIMD                11 ns      10 ns     64000000

Speedup: 8.4x
```

**By String Size:**

| Size (bytes) | Scalar | SIMD | Speedup |
|--------------|--------|------|---------|
| 64 | 45 ns | 8 ns | 5.6x |
| 128 | 88 ns | 12 ns | 7.3x |
| 256 | 175 ns | 18 ns | 9.7x |
| 512 | 350 ns | 28 ns | 12.5x |
| 1024 | 700 ns | 48 ns | 14.6x |
| 4096 | 2800 ns | 165 ns | 17.0x |

**Observation**: SIMD advantage increases with message size due to amortization of setup costs.

### Full Message Parsing Benchmarks

```
Benchmark                              Time        CPU     Iterations
─────────────────────────────────────────────────────────────────────
BM_Parse_Scalar_Small                 320 ns     318 ns      2198763
BM_Parse_SIMD_Small                    52 ns      51 ns     13728459

BM_Parse_Scalar_Medium                452 ns     450 ns      1548271
BM_Parse_SIMD_Medium                   62 ns      60 ns     11286734

BM_Parse_Scalar_Large                 580 ns     578 ns      1210482
BM_Parse_SIMD_Large                    78 ns      76 ns      9178523
```

**Message Size Impact:**

| Message Size | Scalar | SIMD | Speedup |
|--------------|--------|------|---------|
| Small (~60 bytes) | 320 ns | 52 ns | 6.2x |
| Medium (~90 bytes) | 452 ns | 62 ns | 7.3x |
| Large (~150 bytes) | 580 ns | 78 ns | 7.4x |

### Numeric Parsing Benchmarks

```
Benchmark                              Time        CPU     Iterations
─────────────────────────────────────────────────────────────────────
BM_Parse_Int                            5 ns       5 ns    140000000
BM_Parse_Double                        16 ns      16 ns     43750000
```

**Observation**: Numeric parsing is already highly optimized by `std::from_chars`. SIMD optimization here would provide diminishing returns.

### Throughput Benchmarks

```
Benchmark                              Time        CPU     Iterations    Items/s
────────────────────────────────────────────────────────────────────────────────
BM_Throughput_Scalar/10               4.5 us     4.5 us       155827    2.22M
BM_Throughput_SIMD/10                 620 ns     615 ns      1138762   16.26M

BM_Throughput_Scalar/100               45 us      45 us        15583    2.22M
BM_Throughput_SIMD/100                6.2 us     6.1 us       114876   16.39M

BM_Throughput_Scalar/1000            450 us     448 us         1558    2.23M
BM_Throughput_SIMD/1000               62 us      61 us        11487   16.39M

BM_Throughput_Scalar/10000           4.5 ms     4.5 ms          156    2.22M
BM_Throughput_SIMD/10000            620 us     615 us         1138   16.26M
```

---

## Performance Breakdown

### Where Time is Spent (Scalar)

```
┌─────────────────────────────────────────────────────────────┐
│                    Scalar Parse (~450 ns)                    │
├─────────────────────────────────────────────────────────────┤
│  ████████████████████████████████████░░░░░░░░  Delimiter    │
│  (78%) ~350 ns                                  Finding     │
├─────────────────────────────────────────────────────────────┤
│  ████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  Field       │
│  (12%) ~55 ns                                   Extraction  │
├─────────────────────────────────────────────────────────────┤
│  ████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  Tag         │
│  (6%) ~27 ns                                    Dispatch    │
├─────────────────────────────────────────────────────────────┤
│  ██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  Numeric     │
│  (4%) ~18 ns                                    Parsing     │
└─────────────────────────────────────────────────────────────┘
```

### Where Time is Spent (SIMD)

```
┌─────────────────────────────────────────────────────────────┐
│                     SIMD Parse (~60 ns)                      │
├─────────────────────────────────────────────────────────────┤
│  ██████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  Delimiter    │
│  (17%) ~10 ns                                   Finding     │
├─────────────────────────────────────────────────────────────┤
│  ██████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  Field       │
│  (33%) ~20 ns                                   Extraction  │
├─────────────────────────────────────────────────────────────┤
│  ████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  Tag         │
│  (20%) ~12 ns                                   Dispatch    │
├─────────────────────────────────────────────────────────────┤
│  ████████████░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░  Numeric     │
│  (30%) ~18 ns                                   Parsing     │
└─────────────────────────────────────────────────────────────┘
```

**Key Insight**: After SIMD optimization, delimiter finding is no longer the bottleneck. Further optimization would require:
1. SIMD numeric parsing
2. Better field extraction
3. Compile-time tag dispatch

---

## Hardware Requirements

### AVX-512 Compatible Processors

| Vendor | Architecture | Examples | Year |
|--------|--------------|----------|------|
| Intel | Skylake-X | Core X-series (i7-7800X+) | 2017 |
| Intel | Ice Lake | 10th Gen Core (mobile) | 2019 |
| Intel | Tiger Lake | 11th Gen Core | 2020 |
| Intel | Alder Lake | 12th Gen Core (P-cores only) | 2021 |
| Intel | Xeon Scalable | 2nd Gen+ | 2019+ |
| AMD | Zen 4 | Ryzen 7000 series | 2022 |

### Checking AVX-512 Support

**Linux:**
```bash
grep avx512 /proc/cpuinfo
```

**Runtime (in code):**
```cpp
#include "simd_utils.hpp"
if (simd_parser::has_avx512_support()) {
    std::cout << "AVX-512 available\n";
}
```

### Performance on Non-AVX-512 Hardware

Without AVX-512, the parser falls back to scalar implementation automatically:

| CPU | Implementation | Throughput |
|-----|----------------|------------|
| With AVX-512 | SIMD | 12-16M msg/sec |
| Without AVX-512 | Scalar | 2-2.5M msg/sec |

---

## Tuning Guide

### Compiler Flags

**Recommended (Release):**
```cmake
-O3 -march=native -mavx512f -mavx512bw
```

| Flag | Purpose | Impact |
|------|---------|--------|
| `-O3` | Maximum optimization | Required |
| `-march=native` | Use all CPU features | +5-10% |
| `-mavx512f` | Enable AVX-512 Foundation | Required for SIMD |
| `-mavx512bw` | Enable AVX-512 Byte/Word | Required for byte comparison |

**Additional Flags to Consider:**

```bash
-funroll-loops          # May help numeric parsing
-ffast-math             # Faster doubles (use with caution)
-flto                   # Link-time optimization
-fno-exceptions         # If exceptions not used
-fno-rtti               # If RTTI not used
```

### Memory Layout Considerations

**Optimal Input Buffer Alignment:**

```cpp
// Aligned allocation (optional, ~5% improvement)
alignas(64) char buffer[4096];

// Or use aligned_alloc
char* buffer = static_cast<char*>(aligned_alloc(64, 4096));
```

**Note**: The parser uses unaligned loads (`_mm512_loadu_si512`) so alignment is not required, but aligned data may be slightly faster.

### Batch Processing

For maximum throughput, process messages in batches:

```cpp
// Good: Batch processing
std::vector<FIXMessage> results;
results.reserve(batch_size);
for (const auto& msg : messages) {
    results.push_back(parse_simd(msg));
}

// Better: Pre-allocated results
std::vector<FIXMessage> results(batch_size);
for (size_t i = 0; i < batch_size; ++i) {
    results[i] = parse_simd(messages[i]);
}
```

### Avoiding Thermal Throttling

Heavy AVX-512 usage can cause frequency reduction on some CPUs:

1. **Monitor CPU frequency** during benchmarks
2. **Allow cooldown** between benchmark runs
3. **Consider AVX-512 downclocking** in power-sensitive environments

```bash
# Monitor frequency on Linux
watch -n 1 "cat /proc/cpuinfo | grep MHz"
```

---

## Profiling Methodology

### Using Google Benchmark

```bash
# Install
sudo apt-get install libbenchmark-dev

# Build and run
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make benchmark_parser
./bin/benchmark_parser
```

### Using perf

```bash
# Record
perf record -g ./bin/benchmark_parser

# Report
perf report

# Stat counters
perf stat -e cycles,instructions,cache-misses ./bin/benchmark_parser
```

### Key Metrics to Monitor

| Metric | Target | Tool |
|--------|--------|------|
| IPC (Instructions/Cycle) | > 2.0 | `perf stat` |
| Cache Miss Rate | < 1% | `perf stat` |
| Branch Mispredictions | < 1% | `perf stat` |
| Memory Bandwidth | > 1 GB/s | `perf mem` |

### Example perf Output

```
Performance counter stats:

    2,451,234,567  cycles
    5,892,345,678  instructions    # 2.40 IPC
       12,345,678  cache-misses    # 0.21% of refs
        1,234,567  branch-misses   # 0.15% of branches
```

---

## Bottleneck Analysis

### Current Bottlenecks (Post-SIMD Optimization)

1. **Field Extraction (33%)**: Substring operations
2. **Numeric Parsing (30%)**: `std::from_chars` overhead
3. **Tag Dispatch (20%)**: Switch statement execution
4. **Delimiter Finding (17%)**: Already optimized with SIMD

### Future Optimization Opportunities

| Optimization | Estimated Gain | Complexity |
|--------------|----------------|------------|
| SIMD Numeric Parsing | +10-15% | High |
| Perfect Hash Tag Lookup | +5-10% | Medium |
| Vectorized Field Extraction | +5-10% | High |
| Multi-threaded Batch Processing | +4-8x (multi-core) | Medium |

### Amdahl's Law Analysis

With delimiter finding at 17% of runtime:
- Maximum theoretical speedup from further delimiter optimization: 1.2x
- Better to focus on other components

With numeric parsing at 30% of runtime:
- Maximum theoretical speedup from 2x faster numerics: 1.18x
- Worth pursuing for high-volume scenarios

---

## Comparison with Other Parsers

| Parser | Throughput | Notes |
|--------|------------|-------|
| This (SIMD) | 12-16M msg/sec | AVX-512 optimized |
| This (Scalar) | 2-2.5M msg/sec | Baseline |
| QuickFIX | ~500K msg/sec | Full-featured, validated |
| OnixS | ~5M msg/sec | Commercial, optimized |

**Note**: Direct comparisons are difficult due to different feature sets and validation levels.

---

## Summary

### Performance Characteristics

- **Best Case**: Small messages on AVX-512 hardware: ~50 ns/msg (20M msg/sec)
- **Typical Case**: Medium messages on AVX-512 hardware: ~60 ns/msg (16M msg/sec)
- **Fallback Case**: Any message on scalar: ~450 ns/msg (2.2M msg/sec)

### Recommendations

1. **Use `parse_auto()`** for production - handles CPU detection automatically
2. **Process in batches** for maximum throughput
3. **Ensure AVX-512 hardware** for best performance
4. **Profile your specific workload** - message sizes vary by use case
