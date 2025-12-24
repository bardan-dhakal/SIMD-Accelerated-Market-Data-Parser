#pragma once

#include "fix_message.hpp"
#include <string_view>

namespace simd_parser {

/**
 * Parses a FIX protocol message using scalar (non-SIMD) implementation.
 * Serves as baseline for performance comparisons.
 *
 * Iterates through the message character-by-character to find delimiters,
 * then extracts tag=value pairs.
 *
 * @param message FIX message string (tags separated by '|')
 * @return Parsed FIXMessage structure
 */
FIXMessage parse_scalar(std::string_view message);

/**
 * Parses a FIX protocol message using AVX-512 SIMD acceleration.
 *
 * Optimization strategy:
 * 1. Use SIMD to find all delimiters rapidly (64 bytes at once)
 * 2. Split message into fields based on delimiter positions
 * 3. Parse each field to extract tag and value
 * 4. Populate FIXMessage structure with zero-copy string views
 *
 * Performance: ~8x faster than scalar implementation
 *
 * @param message FIX message string (tags separated by '|')
 * @return Parsed FIXMessage structure
 */
FIXMessage parse_simd(std::string_view message);

/**
 * Automatically selects the best parser implementation based on CPU capabilities.
 * Falls back to scalar if AVX-512 is not available.
 *
 * @param message FIX message string
 * @return Parsed FIXMessage structure
 */
FIXMessage parse_auto(std::string_view message);

} // namespace simd_parser
