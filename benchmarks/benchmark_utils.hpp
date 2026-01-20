#pragma once

/**
 * Benchmark Utilities
 *
 * Common test data and utilities for benchmarking the SIMD FIX parser.
 */

#include <string>
#include <vector>
#include <cstdint>

namespace benchmark_utils {

// Small message (~60 bytes) - minimal fields
inline const std::string SMALL_MESSAGE =
    "8=FIX.4.4|35=D|55=SPY|54=1|38=100|44=450.00|";

// Medium message (~90 bytes) - typical order
inline const std::string MEDIUM_MESSAGE =
    "8=FIX.4.4|35=D|49=TRADER1|56=EXCHANGE|55=AAPL|54=1|38=100|44=150.25|";

// Large message (~150 bytes) - extended fields
inline const std::string LARGE_MESSAGE =
    "8=FIX.4.4|35=D|49=QUANTITATIVE_HEDGE_FUND|56=PRIMARY_EXCHANGE_NETWORK|"
    "55=GOOGL|54=1|38=10000|44=141.75|";

// Extra large message (~250 bytes) - stress test
inline const std::string XLARGE_MESSAGE =
    "8=FIX.4.4|35=D|49=INSTITUTIONAL_ASSET_MANAGER_ALPHA|"
    "56=CONSOLIDATED_EXCHANGE_ROUTING_NETWORK|55=BRK.A|54=1|38=5|44=628450.00|"
    "8=FIX.4.4|35=D|49=SECONDARY_TRADER|56=BACKUP_EXCHANGE|55=MSFT|54=2|38=500|44=378.50|";

// Generate a batch of messages for throughput testing
inline std::vector<std::string> generate_message_batch(size_t count) {
    std::vector<std::string> messages;
    messages.reserve(count);

    // Rotate through different symbols and sides
    const char* symbols[] = {"AAPL", "MSFT", "GOOGL", "AMZN", "META",
                             "NVDA", "TSLA", "AMD", "INTC", "JPM"};
    const double prices[] = {150.25, 378.50, 141.75, 178.45, 505.25,
                             875.30, 248.90, 156.80, 45.25, 195.50};

    for (size_t i = 0; i < count; ++i) {
        size_t idx = i % 10;
        int side = (i % 2) + 1;  // Alternate buy/sell
        int qty = static_cast<int>((i % 10 + 1) * 100);

        std::string msg = "8=FIX.4.4|35=D|49=BATCH";
        msg += std::to_string(i);
        msg += "|56=EX|55=";
        msg += symbols[idx];
        msg += "|54=";
        msg += std::to_string(side);
        msg += "|38=";
        msg += std::to_string(qty);
        msg += "|44=";
        msg += std::to_string(prices[idx]);
        msg += "|";

        messages.push_back(std::move(msg));
    }

    return messages;
}

// Generate a string with specific delimiter count for delimiter-finding benchmarks
inline std::string generate_delimiter_string(size_t length, size_t delimiter_count) {
    if (length == 0) return "";
    if (delimiter_count == 0) return std::string(length, 'X');

    std::string result;
    result.reserve(length);

    // Distribute delimiters evenly
    size_t segment_len = length / (delimiter_count + 1);
    if (segment_len == 0) segment_len = 1;

    size_t delims_placed = 0;
    for (size_t i = 0; i < length; ++i) {
        if (delims_placed < delimiter_count &&
            i > 0 &&
            (i % segment_len) == 0) {
            result += '|';
            delims_placed++;
        } else {
            result += 'X';
        }
    }

    return result;
}

// Prevent compiler from optimizing away results
template<typename T>
inline void do_not_optimize(T&& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

// Memory barrier to prevent reordering
inline void clobber_memory() {
    asm volatile("" : : : "memory");
}

} // namespace benchmark_utils
