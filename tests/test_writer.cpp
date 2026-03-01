#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>
#include <fstream>
#include <cstdio>

using namespace edfio;

TEST_CASE("Write and read back header round-trip") {
    // Read original
    std::ifstream instream("Calib5.edf", std::ios::binary);
    REQUIRE(instream.is_open());
    auto header = ReadHeaderExam(instream);
    instream.close();

    // Write to temp file
    const char* tmpfile = "test_roundtrip.edf";
    {
        std::ofstream outstream(tmpfile, std::ios::binary);
        REQUIRE(outstream.is_open());
        WriteHeaderExam(outstream, header);

        // Also write data records
        std::ifstream instream2("Calib5.edf", std::ios::binary);
        auto store = detail::CreateDataRecordStore(instream2, header.m_general);
        auto sink = detail::CreateDataRecordSink(outstream, header.m_general);
        auto sink_it = sink.begin();
        for (auto it = store.begin(); it != store.end(); ++it) {
            *sink_it = *it;
            ++sink_it;
        }
    }

    // Read back
    std::ifstream checkstream(tmpfile, std::ios::binary);
    REQUIRE(checkstream.is_open());
    auto header2 = ReadHeaderExam(checkstream);

    CHECK(header2.m_general.m_totalSignals == header.m_general.m_totalSignals);
    CHECK(header2.m_general.m_datarecordsFile == header.m_general.m_datarecordsFile);
    CHECK(header2.m_general.m_datarecordDuration == doctest::Approx(header.m_general.m_datarecordDuration));
    CHECK(header2.m_signals.size() == header.m_signals.size());

    checkstream.close();
    std::remove(tmpfile);
}
