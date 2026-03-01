#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cstdint>
#include <edfio/EdfIO.hpp>
#include <fstream>
#include <ranges>

using namespace edfio;

// Phase 4: Verify ranges compliance at compile time
static_assert(std::random_access_iterator<DataRecordStore::iterator>);
static_assert(std::random_access_iterator<SignalRecordStore::iterator>);
static_assert(std::bidirectional_iterator<TalStore::iterator>);
static_assert(std::ranges::random_access_range<DataRecordStore>);
static_assert(std::ranges::random_access_range<SignalRecordStore>);
static_assert(std::ranges::bidirectional_range<TalStore>);

TEST_CASE("RecordStore iterator subtraction and arithmetic (multi-record file)") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);
    REQUIRE(store.size() == 600);

    auto a = store.begin();
    auto b = store.begin() + 100;
    auto c = store.end();

    CHECK((b - a) == 100);
    CHECK((a - b) == -100);
    CHECK((c - a) == 600);
    CHECK((a - c) == -600);
}

TEST_CASE("RecordStore iterator n + it and negative offsets") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);
    REQUIRE(store.size() >= 10);

    // n + it form
    auto it = store.begin();
    auto it2 = 5 + it;
    CHECK((it2 - store.begin()) == 5);

    // Negative offset via +=
    auto it3 = store.begin() + 10;
    it3 += -3;
    CHECK((it3 - store.begin()) == 7);

    // Negative offset via -=
    auto it4 = store.begin() + 10;
    it4 -= -2;  // subtracting negative = adding
    CHECK((it4 - store.begin()) == 12);
}

TEST_CASE("DataRecordStore iteration works") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);

    CHECK(store.size() == static_cast<uint64_t>(header.m_general.m_datarecordsFile));
    CHECK(store.size() == 600);

    // Iterate through first 10 records
    uint32_t count = 0;
    for (auto it = store.begin(); it != store.begin() + 10; ++it) {
        auto& rec = *it;
        CHECK(rec.Size() > 0);
        ++count;
    }
    CHECK(count == 10);
}

TEST_CASE("RecordStore iterator comparisons via spaceship") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);
    REQUIRE(store.size() >= 10);

    auto a = store.begin();
    auto b = store.begin() + 5;
    auto c = store.end();
    CHECK(a < b);
    CHECK(b < c);
    CHECK(b > a);
    CHECK(c > b);
    CHECK(a <= a);
    CHECK(a <= b);
    CHECK(c >= b);
    CHECK(a == a);
    CHECK(a != b);
}

TEST_CASE("RecordStore iterator subscript operator") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);
    REQUIRE(store.size() >= 3);

    auto it = store.begin();
    auto& rec0 = it[0];
    auto& rec2 = it[2];
    CHECK(rec0.Size() > 0);
    CHECK(rec2.Size() > 0);
}

TEST_CASE("SignalRecordStore iteration works") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    REQUIRE(header.m_signals.size() > 0);

    auto store = detail::CreateSignalRecordStore(stream, header.m_general, header.m_signals[0]);
    CHECK(store.size() == static_cast<uint64_t>(header.m_general.m_datarecordsFile));

    // Iterate first 5 records
    uint32_t count = 0;
    for (auto it = store.begin(); it != store.begin() + 5; ++it) {
        auto& rec = *it;
        CHECK(rec.Size() > 0);
        ++count;
    }
    CHECK(count == 5);
}

TEST_CASE("DataRecordStore satisfies ranges::random_access_range") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);

    // Use std::ranges algorithms
    CHECK(std::ranges::distance(store.begin(), store.end()) == 600);

    // Range-based for loop (proves range concept works)
    uint32_t count = 0;
    for ([[maybe_unused]] auto& rec : store) {
        ++count;
        if (count >= 3) break;
    }
    CHECK(count == 3);
}

TEST_CASE("Const iteration works without const_cast issues") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);

    // Call const begin/end
    const auto& cstore = store;
    auto it = cstore.begin();
    auto end = cstore.end();
    CHECK(it != end);
    CHECK((end - it) == 600);
    auto& rec = *it;
    CHECK(rec.Size() > 0);
}
