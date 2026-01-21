/**
 * FIX Message Structure Unit Tests
 *
 * Tests for FIXMessage struct initialization and field handling.
 */

#include <gtest/gtest.h>
#include "fix_message.hpp"
#include "parser.hpp"
#include "test_data.hpp"

using namespace simd_parser;

// ============================================================================
// Default Construction Tests
// ============================================================================

TEST(FIXMessageTest, DefaultConstruction) {
    FIXMessage msg;

    EXPECT_TRUE(msg.message_type.empty());
    EXPECT_TRUE(msg.symbol.empty());
    EXPECT_TRUE(msg.sender.empty());
    EXPECT_TRUE(msg.target.empty());
    EXPECT_EQ(msg.side, 0);
    EXPECT_DOUBLE_EQ(msg.price, 0.0);
    EXPECT_EQ(msg.quantity, 0);
    EXPECT_FALSE(msg.valid);
}

TEST(FIXMessageTest, DefaultConstruction_InvalidByDefault) {
    FIXMessage msg;
    EXPECT_FALSE(msg.valid);
}

// ============================================================================
// FIXTag Enum Tests
// ============================================================================

TEST(FIXTagTest, TagValues) {
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::BeginString), 8);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::BodyLength), 9);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::MessageType), 35);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::SenderCompID), 49);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::TargetCompID), 56);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::Side), 54);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::Symbol), 55);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::OrderQty), 38);
    EXPECT_EQ(static_cast<uint32_t>(FIXTag::Price), 44);
}

// ============================================================================
// Field Population Tests
// ============================================================================

TEST(FIXMessageTest, AllFieldsPopulated) {
    auto msg = parse_auto(test_data::valid::FULL_MESSAGE);

    EXPECT_TRUE(msg.valid);
    EXPECT_FALSE(msg.message_type.empty());
    EXPECT_FALSE(msg.symbol.empty());
    EXPECT_FALSE(msg.sender.empty());
    EXPECT_FALSE(msg.target.empty());
    EXPECT_NE(msg.side, 0);
    EXPECT_NE(msg.price, 0.0);
    EXPECT_NE(msg.quantity, 0);
}

TEST(FIXMessageTest, MessageType_NewOrderSingle) {
    auto msg = parse_auto(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_EQ(msg.message_type, "D");
}

TEST(FIXMessageTest, MessageType_ExecutionReport) {
    auto msg = parse_auto(test_data::valid::EXECUTION_REPORT);

    EXPECT_EQ(msg.message_type, "8");
}

TEST(FIXMessageTest, MessageType_OrderCancel) {
    auto msg = parse_auto(test_data::valid::ORDER_CANCEL);

    EXPECT_EQ(msg.message_type, "F");
}

// ============================================================================
// Side Field Tests
// ============================================================================

TEST(FIXMessageTest, Side_Buy) {
    auto msg = parse_auto(test_data::valid::BUY_ORDER);

    EXPECT_EQ(msg.side, 1);
}

TEST(FIXMessageTest, Side_Sell) {
    auto msg = parse_auto(test_data::valid::SELL_ORDER);

    EXPECT_EQ(msg.side, 2);
}

TEST(FIXMessageTest, Side_DefaultZero) {
    // Message without side field
    auto msg = parse_auto("35=D|55=TEST|");

    EXPECT_EQ(msg.side, 0);
}

// ============================================================================
// Symbol Field Tests
// ============================================================================

TEST(FIXMessageTest, Symbol_Standard) {
    auto msg = parse_auto(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_EQ(msg.symbol, "AAPL");
}

TEST(FIXMessageTest, Symbol_Long) {
    auto msg = parse_auto(test_data::valid::LONG_SYMBOL);

    EXPECT_EQ(msg.symbol, "VERYLONGSYMBOLNAME");
}

TEST(FIXMessageTest, Symbol_WithPeriod) {
    auto msg = parse_auto(test_data::valid::HIGH_PRICE);

    EXPECT_EQ(msg.symbol, "BRK.A");
}

// ============================================================================
// Sender/Target Field Tests
// ============================================================================

TEST(FIXMessageTest, SenderTarget_Present) {
    auto msg = parse_auto(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_EQ(msg.sender, "SENDER");
    EXPECT_EQ(msg.target, "TARGET");
}

TEST(FIXMessageTest, SenderTarget_Long) {
    auto msg = parse_auto(test_data::valid::LONG_IDS);

    EXPECT_EQ(msg.sender, "VERY_LONG_SENDER_COMPANY_ID");
    EXPECT_EQ(msg.target, "VERY_LONG_TARGET_COMPANY_ID");
}

TEST(FIXMessageTest, SenderTarget_Missing) {
    auto msg = parse_auto(test_data::valid::MINIMAL);

    EXPECT_TRUE(msg.sender.empty());
    EXPECT_TRUE(msg.target.empty());
}

// ============================================================================
// Price Field Tests
// ============================================================================

TEST(FIXMessageTest, Price_Standard) {
    auto msg = parse_auto(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_DOUBLE_EQ(msg.price, 150.25);
}

TEST(FIXMessageTest, Price_High) {
    auto msg = parse_auto(test_data::valid::HIGH_PRICE);

    EXPECT_DOUBLE_EQ(msg.price, 628450.00);
}

TEST(FIXMessageTest, Price_Low) {
    auto msg = parse_auto(test_data::valid::LOW_PRICE);

    EXPECT_NEAR(msg.price, 0.0025, 0.0001);
}

TEST(FIXMessageTest, Price_Missing) {
    auto msg = parse_auto("35=D|55=TEST|54=1|38=100|");

    EXPECT_DOUBLE_EQ(msg.price, 0.0);
}

// ============================================================================
// Quantity Field Tests
// ============================================================================

TEST(FIXMessageTest, Quantity_Standard) {
    auto msg = parse_auto(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_EQ(msg.quantity, 100);
}

TEST(FIXMessageTest, Quantity_Large) {
    auto msg = parse_auto(test_data::valid::LARGE_QUANTITY);

    EXPECT_EQ(msg.quantity, 999999);
}

TEST(FIXMessageTest, Quantity_Missing) {
    auto msg = parse_auto("35=D|55=TEST|54=1|44=100.00|");

    EXPECT_EQ(msg.quantity, 0);
}

// ============================================================================
// Validity Tests
// ============================================================================

TEST(FIXMessageTest, Valid_WhenTypeAndSymbolPresent) {
    auto msg = parse_auto(test_data::valid::MINIMAL);

    EXPECT_TRUE(msg.valid);
    EXPECT_EQ(msg.message_type, "D");
    EXPECT_EQ(msg.symbol, "SPY");
}

TEST(FIXMessageTest, Invalid_WhenTypeEmpty) {
    auto msg = parse_auto(test_data::invalid::NO_MSG_TYPE);

    EXPECT_FALSE(msg.valid);
}

TEST(FIXMessageTest, Invalid_WhenSymbolEmpty) {
    auto msg = parse_auto(test_data::invalid::NO_SYMBOL);

    EXPECT_FALSE(msg.valid);
}

TEST(FIXMessageTest, Invalid_WhenEmpty) {
    auto msg = parse_auto(test_data::invalid::EMPTY);

    EXPECT_FALSE(msg.valid);
}

// ============================================================================
// String View Behavior Tests
// ============================================================================

TEST(FIXMessageTest, StringView_NotOwned) {
    std::string original = "8=FIX.4.4|35=D|55=AAPL|54=1|";
    auto msg = parse_auto(original);

    // string_view should point into original
    EXPECT_EQ(msg.symbol.data(), original.data() + original.find("AAPL"));
}

TEST(FIXMessageTest, StringView_SizeCorrect) {
    auto msg = parse_auto("8=FIX.4.4|35=D|55=AAPL|");

    EXPECT_EQ(msg.symbol.size(), 4);  // "AAPL"
    EXPECT_EQ(msg.message_type.size(), 1);  // "D"
}

// ============================================================================
// Copy and Move Tests
// ============================================================================

TEST(FIXMessageTest, CopyConstruction) {
    auto original = parse_auto(test_data::valid::NEW_ORDER_SINGLE);
    FIXMessage copy = original;

    EXPECT_EQ(copy.message_type, original.message_type);
    EXPECT_EQ(copy.symbol, original.symbol);
    EXPECT_EQ(copy.side, original.side);
    EXPECT_EQ(copy.price, original.price);
    EXPECT_EQ(copy.quantity, original.quantity);
    EXPECT_EQ(copy.valid, original.valid);
}

TEST(FIXMessageTest, MoveConstruction) {
    auto original = parse_auto(test_data::valid::NEW_ORDER_SINGLE);
    std::string_view orig_symbol = original.symbol;

    FIXMessage moved = std::move(original);

    EXPECT_EQ(moved.symbol, orig_symbol);
    EXPECT_TRUE(moved.valid);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(FIXMessageTest, UnknownTags_Ignored) {
    // Tag 999 is not recognized
    auto msg = parse_auto("35=D|55=TEST|999=UNKNOWN|54=1|");

    EXPECT_TRUE(msg.valid);
    EXPECT_EQ(msg.symbol, "TEST");
    EXPECT_EQ(msg.side, 1);
}

TEST(FIXMessageTest, DuplicateTags_LastWins) {
    // Two symbol fields - last one should be used
    auto msg = parse_auto("35=D|55=FIRST|55=SECOND|");

    EXPECT_TRUE(msg.valid);
    EXPECT_EQ(msg.symbol, "SECOND");
}

TEST(FIXMessageTest, BeginString_Ignored) {
    // BeginString (tag 8) is parsed but not stored
    auto msg = parse_auto("8=FIX.4.4|35=D|55=TEST|");

    EXPECT_TRUE(msg.valid);
    // No field to check for BeginString - it's not stored
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
