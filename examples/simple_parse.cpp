/**
 * Simple Parse Example
 *
 * Demonstrates basic usage of the SIMD-accelerated FIX protocol parser.
 * Shows how to parse FIX messages and extract common fields.
 */

#include "parser.hpp"
#include "simd_utils.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// Helper function to print a separator line
void print_separator(char c = '=', int width = 60) {
    std::cout << std::string(width, c) << "\n";
}

// Helper function to convert side code to human-readable string
const char* side_to_string(int32_t side) {
    switch (side) {
        case 1: return "Buy";
        case 2: return "Sell";
        case 3: return "Buy Minus";
        case 4: return "Sell Plus";
        case 5: return "Sell Short";
        case 6: return "Sell Short Exempt";
        default: return "Unknown";
    }
}

// Helper function to convert message type to human-readable string
const char* message_type_to_string(std::string_view msg_type) {
    if (msg_type == "D") return "New Order - Single";
    if (msg_type == "8") return "Execution Report";
    if (msg_type == "F") return "Order Cancel Request";
    if (msg_type == "G") return "Order Cancel/Replace Request";
    if (msg_type == "0") return "Heartbeat";
    if (msg_type == "A") return "Logon";
    if (msg_type == "5") return "Logout";
    return "Unknown";
}

// Print parsed FIX message details
void print_message(const simd_parser::FIXMessage& msg, int msg_num) {
    std::cout << "\nMessage " << msg_num << ":\n";
    print_separator('-', 40);

    if (!msg.valid) {
        std::cout << "  [INVALID MESSAGE]\n";
        return;
    }

    std::cout << std::left;
    std::cout << "  " << std::setw(12) << "Type:"
              << msg.message_type << " (" << message_type_to_string(msg.message_type) << ")\n";
    std::cout << "  " << std::setw(12) << "Symbol:" << msg.symbol << "\n";

    if (!msg.sender.empty()) {
        std::cout << "  " << std::setw(12) << "Sender:" << msg.sender << "\n";
    }
    if (!msg.target.empty()) {
        std::cout << "  " << std::setw(12) << "Target:" << msg.target << "\n";
    }
    if (msg.side != 0) {
        std::cout << "  " << std::setw(12) << "Side:" << side_to_string(msg.side) << "\n";
    }
    if (msg.quantity != 0) {
        std::cout << "  " << std::setw(12) << "Quantity:" << msg.quantity << "\n";
    }
    if (msg.price != 0.0) {
        std::cout << "  " << std::setw(12) << "Price:" << std::fixed << std::setprecision(2)
                  << "$" << msg.price << "\n";
    }
}

int main() {
    print_separator('=');
    std::cout << "   SIMD-Accelerated Market Data Parser Demo\n";
    print_separator('=');

    // Display CPU capabilities
    std::cout << "\nCPU Features:\n";
    print_separator('-', 40);
    bool avx512 = simd_parser::has_avx512_support();
    std::cout << "  AVX-512 Support: " << (avx512 ? "YES" : "NO") << "\n";
    std::cout << "  Parser Mode:     " << (avx512 ? "SIMD (AVX-512)" : "Scalar") << "\n";

    // Sample FIX messages to parse
    std::vector<std::string> messages = {
        // New Order Single - Buy 100 shares of AAPL at $150.25
        "8=FIX.4.4|35=D|49=TRADER1|56=EXCHANGE|55=AAPL|54=1|38=100|44=150.25|",

        // New Order Single - Sell 500 shares of MSFT at $378.50
        "8=FIX.4.4|35=D|49=TRADER1|56=EXCHANGE|55=MSFT|54=2|38=500|44=378.50|",

        // New Order Single - Buy 1000 shares of GOOGL at $141.75
        "8=FIX.4.4|35=D|49=HEDGE_FUND|56=DARK_POOL|55=GOOGL|54=1|38=1000|44=141.75|",

        // Execution Report
        "8=FIX.4.4|35=8|49=EXCHANGE|56=TRADER1|55=AAPL|54=1|38=100|44=150.25|",

        // Order Cancel Request
        "8=FIX.4.4|35=F|49=TRADER1|56=EXCHANGE|55=TSLA|54=2|38=200|44=248.90|"
    };

    // Parse and display each message using auto-detection
    std::cout << "\n";
    print_separator('=');
    std::cout << "   Parsing Messages (using parse_auto)\n";
    print_separator('=');

    int msg_num = 1;
    for (const auto& raw_msg : messages) {
        // Use parse_auto which automatically selects SIMD or scalar based on CPU
        simd_parser::FIXMessage parsed = simd_parser::parse_auto(raw_msg);
        print_message(parsed, msg_num++);
    }

    // Demonstrate explicit parser selection
    std::cout << "\n";
    print_separator('=');
    std::cout << "   Explicit Parser Selection Demo\n";
    print_separator('=');

    const char* test_msg = "8=FIX.4.4|35=D|49=ALGO_TRADER|56=NYSE|55=NVDA|54=1|38=250|44=875.30|";

    std::cout << "\nRaw message:\n  " << test_msg << "\n";

    // Parse with scalar implementation
    std::cout << "\nUsing parse_scalar():";
    simd_parser::FIXMessage scalar_result = simd_parser::parse_scalar(test_msg);
    print_message(scalar_result, 1);

    // Parse with SIMD implementation (falls back to scalar if AVX-512 not available)
    std::cout << "\nUsing parse_simd():";
    simd_parser::FIXMessage simd_result = simd_parser::parse_simd(test_msg);
    print_message(simd_result, 1);

    // Verify both produce identical results
    std::cout << "\n";
    print_separator('-', 40);
    bool results_match =
        scalar_result.message_type == simd_result.message_type &&
        scalar_result.symbol == simd_result.symbol &&
        scalar_result.sender == simd_result.sender &&
        scalar_result.target == simd_result.target &&
        scalar_result.side == simd_result.side &&
        scalar_result.quantity == simd_result.quantity &&
        scalar_result.price == simd_result.price &&
        scalar_result.valid == simd_result.valid;

    std::cout << "Results match: " << (results_match ? "YES" : "NO") << "\n";

    // Summary
    std::cout << "\n";
    print_separator('=');
    std::cout << "   Summary\n";
    print_separator('=');
    std::cout << "\nTotal messages parsed: " << messages.size() + 2 << "\n";
    std::cout << "Parser implementation: " << (avx512 ? "AVX-512 SIMD" : "Scalar") << "\n";
    std::cout << "\nFor performance benchmarks, run: ./bin/benchmark_parser\n";
    print_separator('=');

    return 0;
}
