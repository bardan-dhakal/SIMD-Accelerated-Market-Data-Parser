#pragma once

#include <string_view>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace simd_parser {

/**
 * Checks if the current CPU supports AVX-512 instructions.
 * Uses CPUID to detect AVX512F and AVX512BW features.
 *
 * @return true if AVX-512 is supported, false otherwise
 */
bool has_avx512_support();

/**
 * Finds all delimiter positions in a string using scalar implementation.
 * Used as baseline for performance comparison.
 *
 * @param data String to search
 * @param delimiter Character to find
 * @return Vector of positions where delimiter occurs
 */
std::vector<size_t> find_delimiters_scalar(std::string_view data, char delimiter);

/**
 * Finds all delimiter positions using AVX-512 SIMD instructions.
 * Processes 64 bytes at a time using vector comparisons.
 *
 * Algorithm:
 * 1. Load 64 bytes into zmm register (_mm512_loadu_si512)
 * 2. Broadcast delimiter to all 64 positions (_mm512_set1_epi8)
 * 3. Compare all bytes simultaneously (_mm512_cmpeq_epi8_mask)
 * 4. Extract matching positions from bitmask
 * 5. Handle remaining bytes (<64) with scalar code
 *
 * @param data String to search
 * @param delimiter Character to find
 * @return Vector of positions where delimiter occurs
 */
std::vector<size_t> find_delimiters_simd(std::string_view data, char delimiter);

/**
 * Parses an integer from a string view without copying.
 * More efficient than std::stoi for small integers.
 *
 * @param str String view containing integer
 * @return Parsed integer value
 */
int32_t parse_int(std::string_view str);

/**
 * Parses a floating point number from a string view without copying.
 * More efficient than std::stod for typical FIX prices.
 *
 * @param str String view containing number
 * @return Parsed double value
 */
double parse_double(std::string_view str);

} // namespace simd_parser
