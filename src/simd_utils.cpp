#include "simd_utils.hpp"
#include <immintrin.h>
#include <cpuid.h>
#include <cstring>
#include <charconv>

namespace simd_parser {

bool has_avx512_support() {
    unsigned int eax, ebx, ecx, edx;

    // Check if CPUID is supported
    if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    // Check for OSXSAVE (bit 27 of ECX)
    if (!(ecx & (1 << 27))) {
        return false;
    }

    // Check for AVX512F support (bit 16 of EBX in leaf 7)
    if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        return false;
    }

    bool has_avx512f = (ebx & (1 << 16)) != 0;
    bool has_avx512bw = (ebx & (1 << 30)) != 0;

    // Check if OS has enabled AVX-512 state saving
    unsigned int xcr0_lo, xcr0_hi;
    __asm__ ("xgetbv" : "=a" (xcr0_lo), "=d" (xcr0_hi) : "c" (0));

    // Bits 5, 6, 7 must be set for AVX-512
    bool os_supports_avx512 = (xcr0_lo & 0xE6) == 0xE6;

    return has_avx512f && has_avx512bw && os_supports_avx512;
}

std::vector<size_t> find_delimiters_scalar(std::string_view data, char delimiter) {
    std::vector<size_t> positions;
    positions.reserve(data.size() / 10);  // Heuristic: assume ~10 chars per field

    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] == delimiter) {
            positions.push_back(i);
        }
    }

    return positions;
}

std::vector<size_t> find_delimiters_simd(std::string_view data, char delimiter) {
    std::vector<size_t> positions;
    positions.reserve(data.size() / 10);

    const char* ptr = data.data();
    const size_t size = data.size();
    size_t pos = 0;

    // Process 64 bytes at a time using AVX-512
    constexpr size_t SIMD_WIDTH = 64;
    const size_t simd_end = size - (size % SIMD_WIDTH);

    // Broadcast delimiter to all 64 positions in zmm register
    const __m512i delim_vec = _mm512_set1_epi8(delimiter);

    for (; pos < simd_end; pos += SIMD_WIDTH) {
        // Load 64 bytes from memory (unaligned load is fine)
        __m512i data_vec = _mm512_loadu_si512(
            reinterpret_cast<const __m512i*>(ptr + pos)
        );

        // Compare all 64 bytes simultaneously
        // Result is a 64-bit mask where each bit indicates a match
        __mmask64 match_mask = _mm512_cmpeq_epi8_mask(data_vec, delim_vec);

        // Extract positions from the bitmask
        while (match_mask != 0) {
            // Find position of lowest set bit (first match in this chunk)
            unsigned int bit_pos = __builtin_ctzll(match_mask);
            positions.push_back(pos + bit_pos);

            // Clear the lowest set bit
            match_mask &= (match_mask - 1);
        }
    }

    // Handle remaining bytes (< 64) with scalar code
    for (; pos < size; ++pos) {
        if (ptr[pos] == delimiter) {
            positions.push_back(pos);
        }
    }

    return positions;
}

int32_t parse_int(std::string_view str) {
    int32_t result = 0;

    // Fast path for std::from_chars (C++17)
    auto [ptr, ec] = std::from_chars(
        str.data(),
        str.data() + str.size(),
        result
    );

    if (ec == std::errc()) {
        return result;
    }

    // Fallback: manual parsing
    bool negative = false;
    size_t i = 0;

    if (!str.empty() && str[0] == '-') {
        negative = true;
        i = 1;
    }

    result = 0;
    for (; i < str.size(); ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        } else {
            break;
        }
    }

    return negative ? -result : result;
}

double parse_double(std::string_view str) {
    double result = 0.0;

    // Try std::from_chars first (C++17, but may not be available for double)
    #ifdef __cpp_lib_to_chars
    auto [ptr, ec] = std::from_chars(
        str.data(),
        str.data() + str.size(),
        result
    );

    if (ec == std::errc()) {
        return result;
    }
    #endif

    // Manual parsing for typical FIX prices (e.g., "150.25")
    bool negative = false;
    size_t i = 0;

    if (!str.empty() && str[0] == '-') {
        negative = true;
        i = 1;
    }

    // Parse integer part
    double integer_part = 0.0;
    for (; i < str.size() && str[i] != '.'; ++i) {
        if (str[i] >= '0' && str[i] <= '9') {
            integer_part = integer_part * 10.0 + (str[i] - '0');
        }
    }

    // Parse fractional part
    double fractional_part = 0.0;
    if (i < str.size() && str[i] == '.') {
        ++i;
        double divisor = 10.0;
        for (; i < str.size(); ++i) {
            if (str[i] >= '0' && str[i] <= '9') {
                fractional_part += (str[i] - '0') / divisor;
                divisor *= 10.0;
            } else {
                break;
            }
        }
    }

    result = integer_part + fractional_part;
    return negative ? -result : result;
}

} // namespace simd_parser
