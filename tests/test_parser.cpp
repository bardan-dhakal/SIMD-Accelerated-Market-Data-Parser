/**
 * Parser Unit Tests
 *
 * Tests for parse_scalar(), parse_simd(), and parse_auto() functions.
 */

#include <gtest/gtest.h>
#include "parser.hpp"
#include "simd_utils.hpp"
#include "test_data.hpp"

using namespace simd_parser;

// ============================================================================
// Test Fixtures
// ============================================================================

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        avx512_available_ = has_avx512_support();
    }

    bool avx512_available_;
};

// ============================================================================
// Basic Parsing Tests
// ============================================================================

TEST_F(ParserTest, ParseScalar_ValidNewOrderSingle) {
    auto result = parse_scalar(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.message_type, "D");
    EXPECT_EQ(result.symbol, "AAPL");
    EXPECT_EQ(result.sender, "SENDER");
    EXPECT_EQ(result.target, "TARGET");
    EXPECT_EQ(result.side, 1);
    EXPECT_EQ(result.quantity, 100);
    EXPECT_DOUBLE_EQ(result.price, 150.25);
}

TEST_F(ParserTest, ParseSIMD_ValidNewOrderSingle) {
    auto result = parse_simd(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.message_type, "D");
    EXPECT_EQ(result.symbol, "AAPL");
    EXPECT_EQ(result.sender, "SENDER");
    EXPECT_EQ(result.target, "TARGET");
    EXPECT_EQ(result.side, 1);
    EXPECT_EQ(result.quantity, 100);
    EXPECT_DOUBLE_EQ(result.price, 150.25);
}

TEST_F(ParserTest, ParseAuto_ValidNewOrderSingle) {
    auto result = parse_auto(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.message_type, "D");
    EXPECT_EQ(result.symbol, "AAPL");
}

// ============================================================================
// Scalar vs SIMD Equivalence Tests
// ============================================================================

TEST_F(ParserTest, ScalarAndSIMD_ProduceSameResults) {
    std::vector<std::string> test_messages = {
        test_data::valid::NEW_ORDER_SINGLE,
        test_data::valid::EXECUTION_REPORT,
        test_data::valid::ORDER_CANCEL,
        test_data::valid::MINIMAL,
        test_data::valid::FULL_MESSAGE,
        test_data::valid::BUY_ORDER,
        test_data::valid::SELL_ORDER,
        test_data::valid::LARGE_QUANTITY,
        test_data::valid::HIGH_PRICE,
        test_data::valid::LOW_PRICE,
    };

    for (const auto& msg : test_messages) {
        auto scalar_result = parse_scalar(msg);
        auto simd_result = parse_simd(msg);

        EXPECT_EQ(scalar_result.valid, simd_result.valid) << "Message: " << msg;
        EXPECT_EQ(scalar_result.message_type, simd_result.message_type) << "Message: " << msg;
        EXPECT_EQ(scalar_result.symbol, simd_result.symbol) << "Message: " << msg;
        EXPECT_EQ(scalar_result.sender, simd_result.sender) << "Message: " << msg;
        EXPECT_EQ(scalar_result.target, simd_result.target) << "Message: " << msg;
        EXPECT_EQ(scalar_result.side, simd_result.side) << "Message: " << msg;
        EXPECT_EQ(scalar_result.quantity, simd_result.quantity) << "Message: " << msg;
        EXPECT_DOUBLE_EQ(scalar_result.price, simd_result.price) << "Message: " << msg;
    }
}

// ============================================================================
// Message Type Tests
// ============================================================================

TEST_F(ParserTest, Parse_ExecutionReport) {
    auto result = parse_auto(test_data::valid::EXECUTION_REPORT);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.message_type, "8");
    EXPECT_EQ(result.symbol, "MSFT");
    EXPECT_EQ(result.side, 2);
}

TEST_F(ParserTest, Parse_OrderCancel) {
    auto result = parse_auto(test_data::valid::ORDER_CANCEL);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.message_type, "F");
    EXPECT_EQ(result.symbol, "GOOGL");
}

// ============================================================================
// Side Field Tests
// ============================================================================

TEST_F(ParserTest, Parse_BuySide) {
    auto result = parse_auto(test_data::valid::BUY_ORDER);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.side, 1);
}

TEST_F(ParserTest, Parse_SellSide) {
    auto result = parse_auto(test_data::valid::SELL_ORDER);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.side, 2);
}

// ============================================================================
// Numeric Field Tests
// ============================================================================

TEST_F(ParserTest, Parse_LargeQuantity) {
    auto result = parse_auto(test_data::valid::LARGE_QUANTITY);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.quantity, 999999);
}

TEST_F(ParserTest, Parse_HighPrice) {
    auto result = parse_auto(test_data::valid::HIGH_PRICE);

    EXPECT_TRUE(result.valid);
    EXPECT_DOUBLE_EQ(result.price, 628450.00);
}

TEST_F(ParserTest, Parse_LowPrice) {
    auto result = parse_auto(test_data::valid::LOW_PRICE);

    EXPECT_TRUE(result.valid);
    EXPECT_NEAR(result.price, 0.0025, 0.0001);
}

// ============================================================================
// Invalid Input Tests
// ============================================================================

TEST_F(ParserTest, Parse_EmptyString) {
    auto result = parse_auto(test_data::invalid::EMPTY);

    EXPECT_FALSE(result.valid);
}

TEST_F(ParserTest, Parse_MissingMessageType) {
    auto result = parse_auto(test_data::invalid::NO_MSG_TYPE);

    EXPECT_FALSE(result.valid);
}

TEST_F(ParserTest, Parse_MissingSymbol) {
    auto result = parse_auto(test_data::invalid::NO_SYMBOL);

    EXPECT_FALSE(result.valid);
}

TEST_F(ParserTest, Parse_MinimalValidMessage) {
    auto result = parse_auto(test_data::valid::MINIMAL);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.message_type, "D");
    EXPECT_EQ(result.symbol, "SPY");
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(ParserTest, Parse_MalformedField_Skipped) {
    // Malformed field should be skipped, but valid fields still parsed
    auto result = parse_auto(test_data::invalid::MALFORMED_FIELD);

    // Should still extract what it can
    EXPECT_EQ(result.symbol, "AAPL");
}

TEST_F(ParserTest, Parse_NoDelimiters) {
    auto result = parse_auto(test_data::invalid::NO_DELIMITERS);

    // Treated as single malformed field
    EXPECT_FALSE(result.valid);
}

TEST_F(ParserTest, Parse_OnlyDelimiters) {
    auto result = parse_auto(test_data::invalid::ONLY_DELIMITERS);

    EXPECT_FALSE(result.valid);
}

TEST_F(ParserTest, Parse_LongSymbol) {
    auto result = parse_auto(test_data::valid::LONG_SYMBOL);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.symbol, "VERYLONGSYMBOLNAME");
}

TEST_F(ParserTest, Parse_LongSenderTarget) {
    auto result = parse_auto(test_data::valid::LONG_IDS);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.sender, "VERY_LONG_SENDER_COMPANY_ID");
    EXPECT_EQ(result.target, "VERY_LONG_TARGET_COMPANY_ID");
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(ParserTest, Parse_LongMessage) {
    std::string long_msg = test_data::invalid::generate_long_message(10);

    // Should not crash
    auto scalar_result = parse_scalar(long_msg);
    auto simd_result = parse_simd(long_msg);

    // Results should match
    EXPECT_EQ(scalar_result.valid, simd_result.valid);
}

TEST_F(ParserTest, Parse_BatchMessages) {
    auto messages = test_data::generate_message_batch(1000);

    for (const auto& msg : messages) {
        auto result = parse_auto(msg);
        EXPECT_TRUE(result.valid) << "Failed on: " << msg;
    }
}

// ============================================================================
// String View Lifetime Tests
// ============================================================================

TEST_F(ParserTest, StringView_PointsToOriginalBuffer) {
    std::string message = "8=FIX.4.4|35=D|55=AAPL|54=1|38=100|44=150.25|";
    auto result = parse_auto(message);

    EXPECT_TRUE(result.valid);

    // Verify string_view points into original buffer
    EXPECT_GE(result.symbol.data(), message.data());
    EXPECT_LT(result.symbol.data(), message.data() + message.size());
    EXPECT_GE(result.message_type.data(), message.data());
    EXPECT_LT(result.message_type.data(), message.data() + message.size());
}

// ============================================================================
// CPU Detection Integration Test
// ============================================================================

TEST_F(ParserTest, ParseAuto_UsesCorrectImplementation) {
    // This is more of an integration test
    // Just verify parse_auto works regardless of CPU
    auto result = parse_auto(test_data::valid::NEW_ORDER_SINGLE);

    EXPECT_TRUE(result.valid);
    EXPECT_EQ(result.symbol, "AAPL");

    if (avx512_available_) {
        // On AVX-512 hardware, both should work
        auto simd_result = parse_simd(test_data::valid::NEW_ORDER_SINGLE);
        EXPECT_TRUE(simd_result.valid);
    }
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
