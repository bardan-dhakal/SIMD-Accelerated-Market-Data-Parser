#include "parser.hpp"
#include "simd_utils.hpp"
#include <algorithm>

namespace simd_parser {

namespace {

/**
 * Splits a FIX field into tag and value.
 * Field format: "tag=value"
 *
 * @param field String view of the field
 * @param tag Output parameter for tag number
 * @param value Output parameter for value string
 * @return true if successfully split, false otherwise
 */
bool split_field(std::string_view field, uint32_t& tag, std::string_view& value) {
    size_t eq_pos = field.find('=');
    if (eq_pos == std::string_view::npos || eq_pos == 0) {
        return false;
    }

    // Extract tag number
    std::string_view tag_str = field.substr(0, eq_pos);
    tag = parse_int(tag_str);

    // Extract value (everything after '=')
    value = field.substr(eq_pos + 1);

    return true;
}

/**
 * Populates a FIXMessage structure from a tag-value pair.
 */
void populate_message(FIXMessage& msg, uint32_t tag, std::string_view value) {
    switch (tag) {
        case static_cast<uint32_t>(FIXTag::MessageType):
            msg.message_type = value;
            break;
        case static_cast<uint32_t>(FIXTag::Symbol):
            msg.symbol = value;
            break;
        case static_cast<uint32_t>(FIXTag::SenderCompID):
            msg.sender = value;
            break;
        case static_cast<uint32_t>(FIXTag::TargetCompID):
            msg.target = value;
            break;
        case static_cast<uint32_t>(FIXTag::Side):
            msg.side = parse_int(value);
            break;
        case static_cast<uint32_t>(FIXTag::Price):
            msg.price = parse_double(value);
            break;
        case static_cast<uint32_t>(FIXTag::OrderQty):
            msg.quantity = parse_int(value);
            break;
        default:
            // Ignore unknown tags
            break;
    }
}

} // anonymous namespace

FIXMessage parse_scalar(std::string_view message) {
    FIXMessage result;

    if (message.empty()) {
        return result;
    }

    // Find all delimiters using scalar implementation
    std::vector<size_t> delimiters = find_delimiters_scalar(message, '|');

    // Parse fields between delimiters
    size_t start = 0;
    for (size_t delim_pos : delimiters) {
        if (delim_pos > start) {
            std::string_view field = message.substr(start, delim_pos - start);

            uint32_t tag;
            std::string_view value;

            if (split_field(field, tag, value)) {
                populate_message(result, tag, value);
            }
        }
        start = delim_pos + 1;
    }

    // Handle last field if message doesn't end with delimiter
    if (start < message.size()) {
        std::string_view field = message.substr(start);

        uint32_t tag;
        std::string_view value;

        if (split_field(field, tag, value)) {
            populate_message(result, tag, value);
        }
    }

    // Validate that we got essential fields
    result.valid = !result.message_type.empty() && !result.symbol.empty();

    return result;
}

FIXMessage parse_simd(std::string_view message) {
    FIXMessage result;

    if (message.empty()) {
        return result;
    }

    // Find all delimiters using SIMD implementation
    std::vector<size_t> delimiters = find_delimiters_simd(message, '|');

    // Parse fields between delimiters
    size_t start = 0;
    for (size_t delim_pos : delimiters) {
        if (delim_pos > start) {
            std::string_view field = message.substr(start, delim_pos - start);

            uint32_t tag;
            std::string_view value;

            if (split_field(field, tag, value)) {
                populate_message(result, tag, value);
            }
        }
        start = delim_pos + 1;
    }

    // Handle last field if message doesn't end with delimiter
    if (start < message.size()) {
        std::string_view field = message.substr(start);

        uint32_t tag;
        std::string_view value;

        if (split_field(field, tag, value)) {
            populate_message(result, tag, value);
        }
    }

    // Validate that we got essential fields
    result.valid = !result.message_type.empty() && !result.symbol.empty();

    return result;
}

FIXMessage parse_auto(std::string_view message) {
    static bool avx512_available = has_avx512_support();

    if (avx512_available) {
        return parse_simd(message);
    } else {
        return parse_scalar(message);
    }
}

} // namespace simd_parser
