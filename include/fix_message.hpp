#pragma once

#include <string_view>
#include <cstdint>

namespace simd_parser {

/**
 * Represents a parsed FIX protocol message.
 * Uses string_view for zero-copy parsing - views point into original message buffer.
 */
struct FIXMessage {
    std::string_view message_type;  // Tag 35: Message type (D=NewOrderSingle, 8=ExecutionReport, etc.)
    std::string_view symbol;        // Tag 55: Symbol/ticker
    std::string_view sender;        // Tag 49: Sender ID
    std::string_view target;        // Tag 56: Target ID
    int32_t side;                   // Tag 54: Side (1=Buy, 2=Sell)
    double price;                   // Tag 44: Price
    int32_t quantity;               // Tag 38: Order quantity

    // Indicates if parsing was successful
    bool valid;

    FIXMessage()
        : side(0), price(0.0), quantity(0), valid(false) {}
};

/**
 * FIX protocol field tags we care about parsing
 */
enum class FIXTag : uint32_t {
    BeginString = 8,     // FIX version
    BodyLength = 9,      // Message body length
    MessageType = 35,    // Type of message
    SenderCompID = 49,   // Sender identifier
    TargetCompID = 56,   // Target identifier
    Side = 54,           // Buy/Sell indicator
    Symbol = 55,         // Trading symbol
    OrderQty = 38,       // Order quantity
    Price = 44,          // Price per unit
};

} // namespace simd_parser
