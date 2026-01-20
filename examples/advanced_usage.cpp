/**
 * Advanced Usage Example
 *
 * Demonstrates advanced features of the SIMD-accelerated FIX parser:
 * - Performance comparison between scalar and SIMD implementations
 * - Batch parsing with throughput measurement
 * - Low-level delimiter finding API usage
 * - Reading and parsing messages from files
 */

#include "parser.hpp"
#include "simd_utils.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>

// Helper function to print a separator line
void print_separator(char c = '=', int width = 70) {
    std::cout << std::string(width, c) << "\n";
}

// Helper function to format large numbers with commas
std::string format_number(uint64_t n) {
    std::string str = std::to_string(n);
    int insert_pos = static_cast<int>(str.length()) - 3;
    while (insert_pos > 0) {
        str.insert(insert_pos, ",");
        insert_pos -= 3;
    }
    return str;
}

// High-resolution timer for performance measurement
class Timer {
public:
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        end_time = std::chrono::high_resolution_clock::now();
    }

    double elapsed_ns() const {
        return std::chrono::duration<double, std::nano>(end_time - start_time).count();
    }

    double elapsed_us() const {
        return std::chrono::duration<double, std::micro>(end_time - start_time).count();
    }

    double elapsed_ms() const {
        return std::chrono::duration<double, std::milli>(end_time - start_time).count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
};

// Demonstrate performance comparison between scalar and SIMD
void demo_performance_comparison() {
    print_separator('=');
    std::cout << "   Performance Comparison: Scalar vs SIMD\n";
    print_separator('=');

    // Test message
    const std::string test_msg =
        "8=FIX.4.4|35=D|49=PERFORMANCE_TEST_SENDER|56=PERFORMANCE_TEST_TARGET|"
        "55=AAPL|54=1|38=1000|44=150.25|";

    const int warmup_iterations = 1000;
    const int test_iterations = 100000;

    std::cout << "\nTest message length: " << test_msg.size() << " bytes\n";
    std::cout << "Warmup iterations:   " << format_number(warmup_iterations) << "\n";
    std::cout << "Test iterations:     " << format_number(test_iterations) << "\n";

    Timer timer;

    // Warmup phase
    std::cout << "\nWarming up...\n";
    for (int i = 0; i < warmup_iterations; ++i) {
        volatile auto r1 = simd_parser::parse_scalar(test_msg);
        volatile auto r2 = simd_parser::parse_simd(test_msg);
        (void)r1;
        (void)r2;
    }

    // Benchmark scalar parsing
    std::cout << "Benchmarking scalar parser...\n";
    timer.start();
    for (int i = 0; i < test_iterations; ++i) {
        volatile auto result = simd_parser::parse_scalar(test_msg);
        (void)result;
    }
    timer.stop();
    double scalar_total_ns = timer.elapsed_ns();
    double scalar_per_msg_ns = scalar_total_ns / test_iterations;

    // Benchmark SIMD parsing
    std::cout << "Benchmarking SIMD parser...\n";
    timer.start();
    for (int i = 0; i < test_iterations; ++i) {
        volatile auto result = simd_parser::parse_simd(test_msg);
        (void)result;
    }
    timer.stop();
    double simd_total_ns = timer.elapsed_ns();
    double simd_per_msg_ns = simd_total_ns / test_iterations;

    // Calculate metrics
    double speedup = scalar_per_msg_ns / simd_per_msg_ns;
    double scalar_throughput = 1e9 / scalar_per_msg_ns;  // messages per second
    double simd_throughput = 1e9 / simd_per_msg_ns;

    // Display results
    std::cout << "\nResults:\n";
    print_separator('-', 70);
    std::cout << std::fixed << std::setprecision(2);
    std::cout << std::left << std::setw(25) << "  Metric"
              << std::right << std::setw(15) << "Scalar"
              << std::setw(15) << "SIMD"
              << std::setw(15) << "Speedup" << "\n";
    print_separator('-', 70);

    std::cout << std::left << std::setw(25) << "  Time per message (ns)"
              << std::right << std::setw(15) << scalar_per_msg_ns
              << std::setw(15) << simd_per_msg_ns
              << std::setw(14) << speedup << "x\n";

    std::cout << std::left << std::setw(25) << "  Throughput (msg/sec)"
              << std::right << std::setw(15) << format_number(static_cast<uint64_t>(scalar_throughput))
              << std::setw(15) << format_number(static_cast<uint64_t>(simd_throughput))
              << std::setw(14) << speedup << "x\n";

    print_separator('-', 70);

    if (simd_parser::has_avx512_support()) {
        std::cout << "\n  Note: AVX-512 is available and being used by SIMD parser.\n";
    } else {
        std::cout << "\n  Note: AVX-512 not available. SIMD parser falls back to scalar.\n";
        std::cout << "        On AVX-512 hardware, expect 6-8x speedup.\n";
    }
}

// Demonstrate low-level delimiter finding API
void demo_delimiter_finding() {
    print_separator('=');
    std::cout << "   Low-Level API: Delimiter Finding\n";
    print_separator('=');

    const std::string test_data =
        "8=FIX.4.4|35=D|49=SENDER|56=TARGET|55=AAPL|54=1|38=100|44=150.25|";

    std::cout << "\nInput string (" << test_data.size() << " bytes):\n";
    std::cout << "  \"" << test_data << "\"\n";

    // Find delimiters using scalar
    auto scalar_positions = simd_parser::find_delimiters_scalar(test_data, '|');

    // Find delimiters using SIMD
    auto simd_positions = simd_parser::find_delimiters_simd(test_data, '|');

    std::cout << "\nDelimiter positions found:\n";
    std::cout << "  Scalar: ";
    for (size_t pos : scalar_positions) {
        std::cout << pos << " ";
    }
    std::cout << "\n";

    std::cout << "  SIMD:   ";
    for (size_t pos : simd_positions) {
        std::cout << pos << " ";
    }
    std::cout << "\n";

    // Verify results match
    bool match = (scalar_positions == simd_positions);
    std::cout << "\n  Results match: " << (match ? "YES" : "NO") << "\n";

    // Show extracted fields
    std::cout << "\nExtracted fields:\n";
    size_t start = 0;
    int field_num = 1;
    for (size_t pos : scalar_positions) {
        std::string_view field(test_data.data() + start, pos - start);
        std::cout << "  Field " << field_num++ << ": \"" << field << "\"\n";
        start = pos + 1;
    }
}

// Demonstrate batch parsing with throughput measurement
void demo_batch_parsing() {
    print_separator('=');
    std::cout << "   Batch Parsing Demo\n";
    print_separator('=');

    // Create a batch of messages
    std::vector<std::string> messages = {
        "8=FIX.4.4|35=D|49=BATCH1|56=EX|55=AAPL|54=1|38=100|44=150.00|",
        "8=FIX.4.4|35=D|49=BATCH2|56=EX|55=MSFT|54=2|38=200|44=375.00|",
        "8=FIX.4.4|35=D|49=BATCH3|56=EX|55=GOOGL|54=1|38=300|44=140.00|",
        "8=FIX.4.4|35=D|49=BATCH4|56=EX|55=AMZN|54=2|38=400|44=175.00|",
        "8=FIX.4.4|35=D|49=BATCH5|56=EX|55=META|54=1|38=500|44=500.00|",
        "8=FIX.4.4|35=D|49=BATCH6|56=EX|55=NVDA|54=2|38=600|44=870.00|",
        "8=FIX.4.4|35=D|49=BATCH7|56=EX|55=TSLA|54=1|38=700|44=245.00|",
        "8=FIX.4.4|35=D|49=BATCH8|56=EX|55=AMD|54=2|38=800|44=155.00|",
        "8=FIX.4.4|35=D|49=BATCH9|56=EX|55=INTC|54=1|38=900|44=45.00|",
        "8=FIX.4.4|35=D|49=BATCH10|56=EX|55=JPM|54=2|38=1000|44=195.00|",
    };

    std::cout << "\nBatch size: " << messages.size() << " messages\n";

    // Parse batch and collect results
    std::vector<simd_parser::FIXMessage> results;
    results.reserve(messages.size());

    Timer timer;
    timer.start();
    for (const auto& msg : messages) {
        results.push_back(simd_parser::parse_auto(msg));
    }
    timer.stop();

    // Display results
    std::cout << "\nParsed messages:\n";
    print_separator('-', 70);
    std::cout << std::left
              << std::setw(8) << "  #"
              << std::setw(10) << "Symbol"
              << std::setw(8) << "Side"
              << std::setw(12) << "Quantity"
              << std::setw(15) << "Price"
              << std::setw(10) << "Valid" << "\n";
    print_separator('-', 70);

    int i = 1;
    for (const auto& result : results) {
        std::cout << std::left
                  << "  " << std::setw(6) << i++
                  << std::setw(10) << result.symbol
                  << std::setw(8) << (result.side == 1 ? "Buy" : "Sell")
                  << std::setw(12) << result.quantity
                  << "$" << std::setw(14) << std::fixed << std::setprecision(2) << result.price
                  << std::setw(10) << (result.valid ? "Yes" : "No") << "\n";
    }

    print_separator('-', 70);
    std::cout << "\n  Batch parsing time: " << std::fixed << std::setprecision(2)
              << timer.elapsed_us() << " microseconds\n";
    std::cout << "  Average per message: " << timer.elapsed_ns() / messages.size()
              << " nanoseconds\n";

    // Calculate totals
    int64_t total_quantity = 0;
    double total_value = 0.0;
    for (const auto& result : results) {
        total_quantity += result.quantity;
        total_value += result.price * result.quantity;
    }

    std::cout << "\n  Total shares: " << format_number(total_quantity) << "\n";
    std::cout << "  Total value:  $" << std::fixed << std::setprecision(2)
              << total_value << "\n";
}

// Demonstrate numeric parsing utilities
void demo_numeric_parsing() {
    print_separator('=');
    std::cout << "   Numeric Parsing Utilities\n";
    print_separator('=');

    // Test integer parsing
    std::vector<std::string> int_tests = {"0", "1", "42", "12345", "-100", "999999"};

    std::cout << "\nInteger parsing (parse_int):\n";
    print_separator('-', 40);
    for (const auto& s : int_tests) {
        int32_t result = simd_parser::parse_int(s);
        std::cout << "  \"" << std::setw(8) << s << "\" -> " << result << "\n";
    }

    // Test double parsing
    std::vector<std::string> double_tests = {
        "0.0", "1.5", "123.456", "9999.99", "-50.25", "0.001"
    };

    std::cout << "\nDouble parsing (parse_double):\n";
    print_separator('-', 40);
    for (const auto& s : double_tests) {
        double result = simd_parser::parse_double(s);
        std::cout << "  \"" << std::setw(10) << s << "\" -> "
                  << std::fixed << std::setprecision(6) << result << "\n";
    }
}

// Read messages from sample file
void demo_file_parsing(const std::string& filename) {
    print_separator('=');
    std::cout << "   File Parsing Demo\n";
    print_separator('=');

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "\n  Could not open file: " << filename << "\n";
        std::cout << "  Skipping file parsing demo.\n";
        return;
    }

    std::cout << "\nReading from: " << filename << "\n";

    std::vector<std::string> messages;
    std::string line;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        messages.push_back(line);
    }
    file.close();

    std::cout << "Found " << messages.size() << " messages in file.\n\n";

    // Parse and display first few messages
    int count = 0;
    const int max_display = 5;

    for (const auto& msg : messages) {
        if (count >= max_display) {
            std::cout << "  ... and " << (messages.size() - max_display) << " more messages\n";
            break;
        }

        auto result = simd_parser::parse_auto(msg);
        if (result.valid) {
            std::cout << "  [" << (count + 1) << "] "
                      << result.symbol << " "
                      << (result.side == 1 ? "BUY" : "SELL") << " "
                      << result.quantity << " @ $"
                      << std::fixed << std::setprecision(2) << result.price << "\n";
        }
        count++;
    }

    // Benchmark parsing all messages
    Timer timer;
    timer.start();
    for (const auto& msg : messages) {
        volatile auto result = simd_parser::parse_auto(msg);
        (void)result;
    }
    timer.stop();

    std::cout << "\n  Parsed " << messages.size() << " messages in "
              << std::fixed << std::setprecision(2) << timer.elapsed_us() << " us\n";
    std::cout << "  Throughput: " << format_number(
        static_cast<uint64_t>(messages.size() * 1e9 / timer.elapsed_ns()))
              << " messages/second\n";
}

int main(int argc, char* argv[]) {
    print_separator('=');
    std::cout << "   SIMD-Accelerated Market Data Parser - Advanced Examples\n";
    print_separator('=');

    // Display CPU capabilities
    std::cout << "\nSystem Information:\n";
    std::cout << "  AVX-512 Support: "
              << (simd_parser::has_avx512_support() ? "YES" : "NO") << "\n";

    // Run demonstrations
    std::cout << "\n";
    demo_performance_comparison();

    std::cout << "\n";
    demo_delimiter_finding();

    std::cout << "\n";
    demo_batch_parsing();

    std::cout << "\n";
    demo_numeric_parsing();

    // Try to find sample_messages.txt
    std::cout << "\n";
    std::vector<std::string> possible_paths = {
        "examples/sample_messages.txt",
        "../examples/sample_messages.txt",
        "sample_messages.txt"
    };

    if (argc > 1) {
        demo_file_parsing(argv[1]);
    } else {
        for (const auto& path : possible_paths) {
            std::ifstream test(path);
            if (test.is_open()) {
                test.close();
                demo_file_parsing(path);
                break;
            }
        }
    }

    std::cout << "\n";
    print_separator('=');
    std::cout << "   Demo Complete\n";
    print_separator('=');

    return 0;
}
