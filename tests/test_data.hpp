#pragma once

/**
 * Test Data and Fixtures
 *
 * Shared test data for all unit tests.
 */

#include <string>
#include <vector>

namespace test_data {

// Valid FIX messages for testing
namespace valid {

// Standard new order single
inline const std::string NEW_ORDER_SINGLE =
    "8=FIX.4.4|35=D|49=SENDER|56=TARGET|55=AAPL|54=1|38=100|44=150.25|";

// Execution report
inline const std::string EXECUTION_REPORT =
    "8=FIX.4.4|35=8|49=EXCHANGE|56=TRADER|55=MSFT|54=2|38=500|44=378.50|";

// Order cancel request
inline const std::string ORDER_CANCEL =
    "8=FIX.4.4|35=F|49=TRADER|56=EXCHANGE|55=GOOGL|54=1|38=200|44=141.75|";

// Minimal valid message (just required fields)
inline const std::string MINIMAL =
    "35=D|55=SPY|";

// Message with all supported fields
inline const std::string FULL_MESSAGE =
    "8=FIX.4.4|9=128|35=D|49=HEDGE_FUND|56=DARK_POOL|55=NVDA|54=2|38=1000|44=875.30|";

// Buy order
inline const std::string BUY_ORDER =
    "8=FIX.4.4|35=D|55=AAPL|54=1|38=100|44=150.00|";

// Sell order
inline const std::string SELL_ORDER =
    "8=FIX.4.4|35=D|55=AAPL|54=2|38=100|44=150.00|";

// Large quantity
inline const std::string LARGE_QUANTITY =
    "8=FIX.4.4|35=D|55=VOO|54=1|38=999999|44=456.78|";

// High price (Berkshire Hathaway)
inline const std::string HIGH_PRICE =
    "8=FIX.4.4|35=D|55=BRK.A|54=1|38=1|44=628450.00|";

// Low price (penny stock)
inline const std::string LOW_PRICE =
    "8=FIX.4.4|35=D|55=PENNY|54=1|38=100000|44=0.0025|";

// Long symbol
inline const std::string LONG_SYMBOL =
    "8=FIX.4.4|35=D|55=VERYLONGSYMBOLNAME|54=1|38=100|44=50.00|";

// Long sender/target IDs
inline const std::string LONG_IDS =
    "8=FIX.4.4|35=D|49=VERY_LONG_SENDER_COMPANY_ID|56=VERY_LONG_TARGET_COMPANY_ID|55=TEST|54=1|38=100|44=100.00|";

}  // namespace valid

// Invalid/edge case messages for testing
namespace invalid {

// Empty message
inline const std::string EMPTY = "";

// Missing message type (tag 35)
inline const std::string NO_MSG_TYPE =
    "8=FIX.4.4|55=AAPL|54=1|38=100|44=150.25|";

// Missing symbol (tag 55)
inline const std::string NO_SYMBOL =
    "8=FIX.4.4|35=D|54=1|38=100|44=150.25|";

// Malformed field (no equals sign)
inline const std::string MALFORMED_FIELD =
    "8=FIX.4.4|35D|55=AAPL|54=1|";

// Empty field value
inline const std::string EMPTY_VALUE =
    "8=FIX.4.4|35=|55=AAPL|";

// No delimiters
inline const std::string NO_DELIMITERS =
    "8=FIX.4.435=D55=AAPL54=1";

// Only delimiters
inline const std::string ONLY_DELIMITERS = "||||||||";

// Single delimiter
inline const std::string SINGLE_DELIMITER = "|";

// Double delimiter
inline const std::string DOUBLE_DELIMITER =
    "8=FIX.4.4||35=D|55=AAPL|";

// Trailing content after delimiter
inline const std::string TRAILING_CONTENT =
    "8=FIX.4.4|35=D|55=AAPL|extra";

// Very long message (stress test)
inline std::string generate_long_message(size_t repeat_count) {
    std::string msg = "8=FIX.4.4|35=D|55=TEST|54=1|38=100|44=50.00|";
    std::string base = msg;
    for (size_t i = 0; i < repeat_count; ++i) {
        msg += base;
    }
    return msg;
}

}  // namespace invalid

// Numeric parsing test cases
namespace numeric {

// Integer test cases: {input, expected_output}
inline const std::vector<std::pair<std::string, int32_t>> INT_CASES = {
    {"0", 0},
    {"1", 1},
    {"42", 42},
    {"123", 123},
    {"12345", 12345},
    {"999999", 999999},
    {"-1", -1},
    {"-42", -42},
    {"-12345", -12345},
    {"2147483647", 2147483647},   // INT32_MAX
    {"-2147483648", -2147483648}, // INT32_MIN
};

// Double test cases: {input, expected_output}
inline const std::vector<std::pair<std::string, double>> DOUBLE_CASES = {
    {"0", 0.0},
    {"0.0", 0.0},
    {"1.0", 1.0},
    {"1.5", 1.5},
    {"123.456", 123.456},
    {"999.99", 999.99},
    {"150.25", 150.25},
    {"0.001", 0.001},
    {"0.0001", 0.0001},
    {"-1.5", -1.5},
    {"-123.456", -123.456},
    {"628450.00", 628450.00},
    {"0.0025", 0.0025},
};

// Edge cases for numeric parsing
inline const std::vector<std::string> INVALID_NUMBERS = {
    "",
    "abc",
    "12.34.56",
    "1,000",
    "1e5",  // Scientific notation (may or may not be supported)
    "++1",
    "--1",
    "1-",
    ".5",   // Leading decimal
    "5.",   // Trailing decimal
};

}  // namespace numeric

// Delimiter finding test cases
namespace delimiters {

// Test cases: {input, delimiter, expected_positions}
struct DelimiterTestCase {
    std::string input;
    char delimiter;
    std::vector<size_t> expected;
};

inline const std::vector<DelimiterTestCase> CASES = {
    // Basic cases
    {"a|b|c", '|', {1, 3}},
    {"|||", '|', {0, 1, 2}},
    {"abc", '|', {}},
    {"", '|', {}},
    {"|", '|', {0}},
    {"a|", '|', {1}},
    {"|a", '|', {0}},

    // Multiple delimiters
    {"a|b|c|d|e", '|', {1, 3, 5, 7}},

    // Long strings (for SIMD testing - 64+ bytes)
    {"0123456789|0123456789|0123456789|0123456789|0123456789|0123456789|", '|',
     {10, 21, 32, 43, 54, 65}},

    // Different delimiter
    {"a,b,c", ',', {1, 3}},
    {"a=b=c", '=', {1, 3}},
    {"a\x01b\x01c", '\x01', {1, 3}},  // SOH delimiter (real FIX)
};

// Generate string of specific length with evenly distributed delimiters
inline std::string generate_test_string(size_t length, size_t delimiter_count, char delim = '|') {
    if (length == 0) return "";
    if (delimiter_count == 0) return std::string(length, 'X');

    std::string result;
    result.reserve(length);

    size_t segment_len = length / (delimiter_count + 1);
    if (segment_len == 0) segment_len = 1;

    size_t delims_placed = 0;
    for (size_t i = 0; i < length; ++i) {
        if (delims_placed < delimiter_count && i > 0 && (i % segment_len) == 0) {
            result += delim;
            delims_placed++;
        } else {
            result += 'X';
        }
    }

    return result;
}

}  // namespace delimiters

// Batch of messages for throughput testing
inline std::vector<std::string> generate_message_batch(size_t count) {
    std::vector<std::string> messages;
    messages.reserve(count);

    const char* symbols[] = {"AAPL", "MSFT", "GOOGL", "AMZN", "META"};
    const double prices[] = {150.25, 378.50, 141.75, 178.45, 505.25};

    for (size_t i = 0; i < count; ++i) {
        size_t idx = i % 5;
        int side = (i % 2) + 1;
        int qty = static_cast<int>((i % 10 + 1) * 100);

        std::string msg = "8=FIX.4.4|35=D|49=TEST|56=EXCH|55=";
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

}  // namespace test_data
