#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>
#include <fstream>

using namespace edfio;

TEST_CASE("Read Calib5.edf header successfully") {
    std::ifstream stream("Calib5.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    ReaderHeaderExam reader;
    auto header = reader(stream);

    CHECK(header.m_general.m_totalSignals > 0);
    CHECK(header.m_general.m_datarecordsFile > 0);
    CHECK(header.m_general.m_datarecordDuration > 0);
    CHECK(header.m_general.m_headerSize > 0);
    CHECK(header.m_signals.size() == static_cast<size_t>(header.m_general.m_totalSignals));
}

TEST_CASE("Calib5.edf is plain EDF format") {
    std::ifstream stream("Calib5.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    ReaderHeaderExam reader;
    auto header = reader(stream);

    CHECK(IsEdf(header.m_general.m_version));
    CHECK_FALSE(IsBdf(header.m_general.m_version));
}

TEST_CASE("Calib5.edf signal headers are valid") {
    std::ifstream stream("Calib5.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    ReaderHeaderExam reader;
    auto header = reader(stream);

    for (auto const& sig : header.m_signals) {
        CHECK(sig.m_samplesInDataRecord > 0);
        CHECK(sig.m_digitalMax > sig.m_digitalMin);
    }
}
