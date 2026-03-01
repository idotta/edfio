#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>

TEST_CASE("ReduceString trims and collapses spaces") {
    using edfio::detail::ReduceString;
    CHECK(ReduceString("  hello   world  ") == "hello world");
    CHECK(ReduceString("   ") == "");
    CHECK(ReduceString("no_change") == "no_change");
    CHECK(ReduceString(" a  b  c ") == "a b c");
    CHECK(ReduceString("") == "");
    CHECK(ReduceString("single") == "single");
    CHECK(ReduceString("  leading") == "leading");
    CHECK(ReduceString("trailing  ") == "trailing");
}

TEST_CASE("GetStringFromMonth handles boundaries") {
    CHECK(edfio::detail::GetStringFromMonth(0) == "JAN");  // was UB, now returns default
    CHECK(edfio::detail::GetStringFromMonth(1) == "JAN");
    CHECK(edfio::detail::GetStringFromMonth(6) == "JUN");
    CHECK(edfio::detail::GetStringFromMonth(12) == "DEC");
    CHECK(edfio::detail::GetStringFromMonth(13) == "JAN");  // out of range, returns default
}

TEST_CASE("GetMonthFromString returns correct values") {
    CHECK(edfio::detail::GetMonthFromString("JAN") == 1);
    CHECK(edfio::detail::GetMonthFromString("DEC") == 12);
    CHECK(edfio::detail::GetMonthFromString("XXX") == 0);
}

TEST_CASE("GetFormatName returns correct strings") {
    CHECK(edfio::detail::GetFormatName(edfio::DataFormat::Edf) == "EDF");
    CHECK(edfio::detail::GetFormatName(edfio::DataFormat::BdfPlusD) == "BDF+D");
    CHECK(edfio::detail::GetFormatName(edfio::DataFormat::Invalid) == "");
}

TEST_CASE("DataFormat constexpr functions") {
    static_assert(edfio::IsEdf(edfio::DataFormat::Edf));
    static_assert(!edfio::IsBdf(edfio::DataFormat::Edf));
    static_assert(edfio::IsBdf(edfio::DataFormat::Bdf));
    static_assert(edfio::IsPlus(edfio::DataFormat::EdfPlusC));
    static_assert(!edfio::IsPlus(edfio::DataFormat::Edf));
    static_assert(edfio::GetSampleBytes(edfio::DataFormat::Edf) == 2);
    static_assert(edfio::GetSampleBytes(edfio::DataFormat::Bdf) == 3);
    CHECK(true);
}

TEST_CASE("GetMonthFromString accepts string_view") {
    using namespace std::string_view_literals;
    CHECK(edfio::detail::GetMonthFromString("FEB"sv) == 2);
    CHECK(edfio::detail::GetMonthFromString("NOV"sv) == 11);
}

TEST_CASE("GetFormatName returns string_view") {
    auto name = edfio::detail::GetFormatName(edfio::DataFormat::EdfPlusC);
    static_assert(std::is_same_v<decltype(name), std::string_view>);
    CHECK(name == "EDF+C");
}

TEST_CASE("ParseInt parses integers and trims spaces") {
    using edfio::detail::ParseInt;
    CHECK(ParseInt("42", "err") == 42);
    CHECK(ParseInt("  42  ", "err") == 42);
    CHECK(ParseInt("-7", "err") == -7);
    CHECK(ParseInt("0", "err") == 0);
    CHECK_THROWS_AS(ParseInt("", "bad"), std::invalid_argument);
    CHECK_THROWS_AS(ParseInt("abc", "bad"), std::invalid_argument);
    CHECK_THROWS_AS(ParseInt("   ", "bad"), std::invalid_argument);
}

TEST_CASE("ParseLongLong parses large integers") {
    using edfio::detail::ParseLongLong;
    CHECK(ParseLongLong("600", "err") == 600LL);
    CHECK(ParseLongLong("  12345678901  ", "err") == 12345678901LL);
    CHECK_THROWS_AS(ParseLongLong("xyz", "bad"), std::invalid_argument);
}

TEST_CASE("ParseDouble parses floating-point numbers") {
    using edfio::detail::ParseDouble;
    CHECK(ParseDouble("3.14", "err") == doctest::Approx(3.14));
    CHECK(ParseDouble("  1.0  ", "err") == doctest::Approx(1.0));
    CHECK(ParseDouble("-2.5", "err") == doctest::Approx(-2.5));
    CHECK(ParseDouble("+0.001", "err") == doctest::Approx(0.001));
    CHECK_THROWS_AS(ParseDouble("abc", "bad"), std::invalid_argument);
}

TEST_CASE("GetError returns non-null for all error codes") {
    CHECK(edfio::GetError(edfio::FileErrc::FileDoesNotOpen) != nullptr);
    CHECK(edfio::GetError(edfio::FileErrc::FileNotOpened) != nullptr);
    CHECK(edfio::GetError(edfio::FileErrc::FileReadError) != nullptr);
    CHECK(edfio::GetError(edfio::FileErrc::FileContainsFormatErrors) != nullptr);
    CHECK(edfio::GetError(edfio::FileErrc::FileWriteError) != nullptr);
}
