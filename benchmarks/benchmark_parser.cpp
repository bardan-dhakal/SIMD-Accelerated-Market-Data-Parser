/**
 * SIMD Market Data Parser Benchmarks
 *
 * Comprehensive benchmarks using Google Benchmark to measure:
 * - Scalar vs SIMD delimiter finding
 * - Scalar vs SIMD full message parsing
 * - Numeric parsing performance
 * - Throughput for various message sizes
 * - Batch processing performance
 */

#include <benchmark/benchmark.h>
#include "parser.hpp"
#include "simd_utils.hpp"
#include "benchmark_utils.hpp"
#include <iostream>
#include <iomanip>

using namespace simd_parser;
using namespace benchmark_utils;

// ============================================================================
// DELIMITER FINDING BENCHMARKS
// ============================================================================

// Benchmark scalar delimiter finding
static void BM_Find_Delimiters_Scalar(benchmark::State& state) {
    const std::string& msg = MEDIUM_MESSAGE;

    for (auto _ : state) {
        auto positions = find_delimiters_scalar(msg, '|');
        benchmark::DoNotOptimize(positions);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Find_Delimiters_Scalar);

// Benchmark SIMD delimiter finding
static void BM_Find_Delimiters_SIMD(benchmark::State& state) {
    const std::string& msg = MEDIUM_MESSAGE;

    for (auto _ : state) {
        auto positions = find_delimiters_simd(msg, '|');
        benchmark::DoNotOptimize(positions);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Find_Delimiters_SIMD);

// Benchmark delimiter finding with varying string sizes
static void BM_Find_Delimiters_Scalar_Size(benchmark::State& state) {
    const size_t size = state.range(0);
    std::string data = generate_delimiter_string(size, size / 10);

    for (auto _ : state) {
        auto positions = find_delimiters_scalar(data, '|');
        benchmark::DoNotOptimize(positions);
    }

    state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_Find_Delimiters_Scalar_Size)
    ->Arg(64)
    ->Arg(128)
    ->Arg(256)
    ->Arg(512)
    ->Arg(1024)
    ->Arg(4096);

static void BM_Find_Delimiters_SIMD_Size(benchmark::State& state) {
    const size_t size = state.range(0);
    std::string data = generate_delimiter_string(size, size / 10);

    for (auto _ : state) {
        auto positions = find_delimiters_simd(data, '|');
        benchmark::DoNotOptimize(positions);
    }

    state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_Find_Delimiters_SIMD_Size)
    ->Arg(64)
    ->Arg(128)
    ->Arg(256)
    ->Arg(512)
    ->Arg(1024)
    ->Arg(4096);

// ============================================================================
// FULL MESSAGE PARSING BENCHMARKS
// ============================================================================

// Benchmark scalar parsing - small message
static void BM_Parse_Scalar_Small(benchmark::State& state) {
    const std::string& msg = SMALL_MESSAGE;

    for (auto _ : state) {
        auto result = parse_scalar(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Scalar_Small);

// Benchmark SIMD parsing - small message
static void BM_Parse_SIMD_Small(benchmark::State& state) {
    const std::string& msg = SMALL_MESSAGE;

    for (auto _ : state) {
        auto result = parse_simd(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_SIMD_Small);

// Benchmark scalar parsing - medium message
static void BM_Parse_Scalar_Medium(benchmark::State& state) {
    const std::string& msg = MEDIUM_MESSAGE;

    for (auto _ : state) {
        auto result = parse_scalar(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Scalar_Medium);

// Benchmark SIMD parsing - medium message
static void BM_Parse_SIMD_Medium(benchmark::State& state) {
    const std::string& msg = MEDIUM_MESSAGE;

    for (auto _ : state) {
        auto result = parse_simd(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_SIMD_Medium);

// Benchmark scalar parsing - large message
static void BM_Parse_Scalar_Large(benchmark::State& state) {
    const std::string& msg = LARGE_MESSAGE;

    for (auto _ : state) {
        auto result = parse_scalar(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Scalar_Large);

// Benchmark SIMD parsing - large message
static void BM_Parse_SIMD_Large(benchmark::State& state) {
    const std::string& msg = LARGE_MESSAGE;

    for (auto _ : state) {
        auto result = parse_simd(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_SIMD_Large);

// Benchmark auto-detection parsing (typical usage)
static void BM_Parse_Auto(benchmark::State& state) {
    const std::string& msg = MEDIUM_MESSAGE;

    for (auto _ : state) {
        auto result = parse_auto(msg);
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * msg.size());
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Auto);

// ============================================================================
// NUMERIC PARSING BENCHMARKS
// ============================================================================

// Benchmark integer parsing
static void BM_Parse_Int(benchmark::State& state) {
    std::string_view int_str = "12345";

    for (auto _ : state) {
        auto result = parse_int(int_str);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Int);

// Benchmark integer parsing - various sizes
static void BM_Parse_Int_Size(benchmark::State& state) {
    const int digits = state.range(0);
    std::string int_str;
    for (int i = 0; i < digits; ++i) {
        int_str += '0' + ((i + 1) % 10);
    }

    for (auto _ : state) {
        auto result = parse_int(int_str);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Int_Size)->Arg(1)->Arg(3)->Arg(5)->Arg(7)->Arg(9);

// Benchmark double parsing
static void BM_Parse_Double(benchmark::State& state) {
    std::string_view double_str = "12345.67";

    for (auto _ : state) {
        auto result = parse_double(double_str);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Double);

// Benchmark double parsing - various precision
static void BM_Parse_Double_Precision(benchmark::State& state) {
    const int precision = state.range(0);
    std::string double_str = "12345.";
    for (int i = 0; i < precision; ++i) {
        double_str += '0' + ((i + 1) % 10);
    }

    for (auto _ : state) {
        auto result = parse_double(double_str);
        benchmark::DoNotOptimize(result);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Parse_Double_Precision)->Arg(1)->Arg(2)->Arg(4)->Arg(6)->Arg(8);

// ============================================================================
// THROUGHPUT BENCHMARKS
// ============================================================================

// Batch throughput - scalar
static void BM_Throughput_Scalar(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto messages = generate_message_batch(batch_size);

    for (auto _ : state) {
        for (const auto& msg : messages) {
            auto result = parse_scalar(msg);
            benchmark::DoNotOptimize(result);
        }
    }

    state.SetItemsProcessed(state.iterations() * batch_size);
}
BENCHMARK(BM_Throughput_Scalar)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// Batch throughput - SIMD
static void BM_Throughput_SIMD(benchmark::State& state) {
    const size_t batch_size = state.range(0);
    auto messages = generate_message_batch(batch_size);

    for (auto _ : state) {
        for (const auto& msg : messages) {
            auto result = parse_simd(msg);
            benchmark::DoNotOptimize(result);
        }
    }

    state.SetItemsProcessed(state.iterations() * batch_size);
}
BENCHMARK(BM_Throughput_SIMD)
    ->Arg(10)
    ->Arg(100)
    ->Arg(1000)
    ->Arg(10000);

// ============================================================================
// LATENCY PERCENTILE BENCHMARKS
// ============================================================================

// Measure parsing latency distribution
static void BM_Latency_Scalar(benchmark::State& state) {
    const std::string& msg = MEDIUM_MESSAGE;

    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = parse_scalar(msg);
        auto end = std::chrono::high_resolution_clock::now();

        benchmark::DoNotOptimize(result);

        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        state.SetIterationTime(elapsed.count() / 1e9);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Latency_Scalar)->UseManualTime();

static void BM_Latency_SIMD(benchmark::State& state) {
    const std::string& msg = MEDIUM_MESSAGE;

    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = parse_simd(msg);
        auto end = std::chrono::high_resolution_clock::now();

        benchmark::DoNotOptimize(result);

        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
        state.SetIterationTime(elapsed.count() / 1e9);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Latency_SIMD)->UseManualTime();

// ============================================================================
// CPU DETECTION BENCHMARK
// ============================================================================

static void BM_CPU_Detection(benchmark::State& state) {
    for (auto _ : state) {
        bool result = has_avx512_support();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CPU_Detection);

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char** argv) {
    // Print header
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "     SIMD Market Data Parser Benchmarks\n";
    std::cout << "============================================================\n";
    std::cout << "\n";

    // Print CPU info
    std::cout << "CPU Features:\n";
    std::cout << "  AVX-512 Support: " << (has_avx512_support() ? "YES" : "NO") << "\n";
    std::cout << "\n";

    // Print test message sizes
    std::cout << "Test Message Sizes:\n";
    std::cout << "  Small:  " << SMALL_MESSAGE.size() << " bytes\n";
    std::cout << "  Medium: " << MEDIUM_MESSAGE.size() << " bytes\n";
    std::cout << "  Large:  " << LARGE_MESSAGE.size() << " bytes\n";
    std::cout << "  XLarge: " << XLARGE_MESSAGE.size() << " bytes\n";
    std::cout << "\n";

    std::cout << "============================================================\n";
    std::cout << "\n";

    // Initialize and run benchmarks
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    benchmark::Shutdown();

    // Print summary
    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "     Benchmark Complete\n";
    std::cout << "============================================================\n";
    std::cout << "\n";
    std::cout << "Key metrics to look for:\n";
    std::cout << "  - BM_Find_Delimiters_SIMD vs BM_Find_Delimiters_Scalar\n";
    std::cout << "    Expected speedup: 8-10x on AVX-512 hardware\n";
    std::cout << "\n";
    std::cout << "  - BM_Parse_SIMD_* vs BM_Parse_Scalar_*\n";
    std::cout << "    Expected speedup: 6-8x on AVX-512 hardware\n";
    std::cout << "\n";
    std::cout << "  - BM_Throughput_SIMD vs BM_Throughput_Scalar\n";
    std::cout << "    Expected: 12-16M msg/sec (SIMD) vs 2-2.5M msg/sec (scalar)\n";
    std::cout << "\n";

    return 0;
}
