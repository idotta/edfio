#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>
#include <fstream>
#include <cstdio>

using namespace edfio;

// BDF 24-bit signed integer limits
static constexpr int32_t BDF_DIGITAL_MIN = -8388608;  // -(2^23)
static constexpr int32_t BDF_DIGITAL_MAX =  8388607;  //  (2^23) - 1

// ---------------------------------------------------------------------------
// 1. Read BDF header successfully
// ---------------------------------------------------------------------------
TEST_CASE("Read BDF header successfully") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);

    CHECK(header.m_general.m_totalSignals > 0);
    CHECK(header.m_general.m_headerSize > 0);
    CHECK(header.m_general.m_datarecordsFile > 0);
    CHECK(header.m_general.m_datarecordDuration > 0.0);
    CHECK(header.m_signals.size() ==
          static_cast<size_t>(header.m_general.m_totalSignals));
}

// ---------------------------------------------------------------------------
// 2. BDF format detection
// ---------------------------------------------------------------------------
TEST_CASE("BDF format detection") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);

    CHECK(IsBdf(header.m_general.m_version));
    CHECK_FALSE(IsEdf(header.m_general.m_version));
    CHECK(GetSampleBytes(header.m_general.m_version) == 3);
}

// ---------------------------------------------------------------------------
// 3. BDF signal headers valid
// ---------------------------------------------------------------------------
TEST_CASE("BDF signal headers valid") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);
    REQUIRE(header.m_signals.size() > 0);

    for (auto const &sig : header.m_signals) {
        CHECK(sig.m_samplesInDataRecord > 0);
        CHECK(sig.m_digitalMax > sig.m_digitalMin);
        // BDF digital range must fit within 24-bit signed bounds
        CHECK(sig.m_digitalMin >= BDF_DIGITAL_MIN);
        CHECK(sig.m_digitalMax <= BDF_DIGITAL_MAX);
        // Physical range must be valid
        CHECK(sig.m_physicalMax > sig.m_physicalMin);
        // Label should not be empty
        CHECK_FALSE(sig.m_label.empty());
    }
}

// ---------------------------------------------------------------------------
// 4. BDF data record iteration
// ---------------------------------------------------------------------------
TEST_CASE("BDF data record iteration") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);
    auto store = detail::CreateDataRecordStore(stream, header.m_general);

    CHECK(store.size() ==
          static_cast<uint64_t>(header.m_general.m_datarecordsFile));
    CHECK(store.size() > 0);

    // Iterate the first few data records and verify they have content
    uint32_t count = 0;
    auto limit = std::min(store.size(),
                          static_cast<decltype(store.size())>(5));
    for (auto it = store.begin(); it != store.begin() + static_cast<int64_t>(limit); ++it) {
        auto const &rec = *it;
        CHECK(rec.Size() > 0);
        // Each data record should be recordSize bytes
        CHECK(rec.Size() == header.m_general.m_detail.m_recordSize);
        ++count;
    }
    CHECK(count == limit);
}

// ---------------------------------------------------------------------------
// 5. BDF signal record iteration
// ---------------------------------------------------------------------------
TEST_CASE("BDF signal record iteration") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);
    REQUIRE(header.m_signals.size() > 0);

    auto const &sig = header.m_signals[0];
    auto store = detail::CreateSignalRecordStore(stream, header.m_general, sig);

    CHECK(store.size() ==
          static_cast<uint64_t>(header.m_general.m_datarecordsFile));

    // Iterate first few signal records
    uint32_t count = 0;
    auto limit = std::min(store.size(),
                          static_cast<decltype(store.size())>(5));
    for (auto it = store.begin(); it != store.begin() + static_cast<int64_t>(limit); ++it) {
        auto const &rec = *it;
        CHECK(rec.Size() > 0);
        // Signal record size = samplesInDataRecord * sampleBytes
        auto expectedSize = static_cast<size_t>(sig.m_samplesInDataRecord) *
                            static_cast<size_t>(GetSampleBytes(header.m_general.m_version));
        CHECK(rec.Size() == expectedSize);
        ++count;
    }
    CHECK(count == limit);
}

// ---------------------------------------------------------------------------
// 6. BDF signal sample store
// ---------------------------------------------------------------------------
TEST_CASE("BDF signal sample store") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);
    REQUIRE(header.m_signals.size() > 0);

    auto const &sig = header.m_signals[0];
    auto sampleStore = detail::CreateSignalSampleStore(stream, header.m_general, sig);

    auto expectedSize = static_cast<uint64_t>(header.m_general.m_datarecordsFile) *
                        static_cast<uint64_t>(sig.m_samplesInDataRecord);
    CHECK(sampleStore.size() == expectedSize);
    CHECK(sampleStore.size() > 0);
}

// ---------------------------------------------------------------------------
// 7. BDF sample reading - digital values in valid 24-bit signed range
// ---------------------------------------------------------------------------
TEST_CASE("BDF sample reading - digital values in 24-bit range") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);
    REQUIRE(header.m_signals.size() > 0);

    auto const &sig = header.m_signals[0];
    auto sampleStore = detail::CreateSignalSampleStore(stream, header.m_general, sig);
    REQUIRE(sampleStore.size() > 0);

    // Digital processor: offset=0, scaling=1 gives raw digital value
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);

    // Read the first N samples and verify they are within 24-bit signed range
    auto limit = std::min(sampleStore.size(),
                          static_cast<decltype(sampleStore.size())>(100));
    for (auto it = sampleStore.begin();
         it != sampleStore.begin() + static_cast<int64_t>(limit); ++it) {
        auto const &sampleRec = *it;
        CHECK(sampleRec.Size() == 3);  // BDF: 3 bytes per sample

        int32_t digitalVal = proc(sampleRec);
        CHECK(digitalVal >= BDF_DIGITAL_MIN);
        CHECK(digitalVal <= BDF_DIGITAL_MAX);
    }
}

// ---------------------------------------------------------------------------
// 8. BDF physical sample conversion
// ---------------------------------------------------------------------------
TEST_CASE("BDF physical sample conversion") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());

    auto header = ReadHeaderExam(stream);
    REQUIRE(header.m_signals.size() > 0);

    auto const &sig = header.m_signals[0];
    auto sampleStore = detail::CreateSignalSampleStore(stream, header.m_general, sig);
    REQUIRE(sampleStore.size() > 0);

    // Physical processor uses the signal's calibration parameters
    ProcessorSampleRecord<SampleType::Physical> physProc(
        sig.m_detail.m_offset, sig.m_detail.m_scaling);

    // Verify scaling and offset are reasonable
    CHECK(sig.m_detail.m_scaling != 0.0);

    // Read the first N samples and verify physical values fall within the
    // declared physical range (with a small tolerance for rounding)
    double physMin = sig.m_physicalMin;
    double physMax = sig.m_physicalMax;
    double tolerance = (physMax - physMin) * 0.001;  // 0.1% tolerance
    double lowerBound = physMin - tolerance;
    double upperBound = physMax + tolerance;

    auto limit = std::min(sampleStore.size(),
                          static_cast<decltype(sampleStore.size())>(100));
    for (auto it = sampleStore.begin();
         it != sampleStore.begin() + static_cast<int64_t>(limit); ++it) {
        auto const &sampleRec = *it;
        double physVal = physProc(sampleRec);
        CHECK(physVal >= lowerBound);
        CHECK(physVal <= upperBound);
    }
}

// ---------------------------------------------------------------------------
// 9. BDF write and read-back round-trip
// ---------------------------------------------------------------------------
TEST_CASE("BDF write and read-back round-trip") {
    // Read original BDF file
    std::ifstream instream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(instream.is_open());
    auto header = ReadHeaderExam(instream);
    instream.close();

    const char *tmpfile = "test_roundtrip.bdf";

    // Write header and all data records to a temporary file
    {
        std::ofstream outstream(tmpfile, std::ios::binary);
        REQUIRE(outstream.is_open());
        WriteHeaderExam(outstream, header);

        // Re-open the original for data copy
        std::ifstream instream2("test_generator_2.bdf", std::ios::binary);
        REQUIRE(instream2.is_open());

        auto store = detail::CreateDataRecordStore(instream2, header.m_general);

        // Write each data record directly to the output stream
        for (auto it = store.begin(); it != store.end(); ++it) {
            outstream << *it;
        }
    }

    // Read back and compare
    std::ifstream checkstream(tmpfile, std::ios::binary);
    REQUIRE(checkstream.is_open());
    auto header2 = ReadHeaderExam(checkstream);

    // General header fields must match
    CHECK(header2.m_general.m_version == header.m_general.m_version);
    CHECK(IsBdf(header2.m_general.m_version));
    CHECK(header2.m_general.m_totalSignals == header.m_general.m_totalSignals);
    CHECK(header2.m_general.m_datarecordsFile == header.m_general.m_datarecordsFile);
    CHECK(header2.m_general.m_datarecordDuration ==
          doctest::Approx(header.m_general.m_datarecordDuration));
    CHECK(header2.m_general.m_headerSize == header.m_general.m_headerSize);
    CHECK(header2.m_general.m_detail.m_recordSize ==
          header.m_general.m_detail.m_recordSize);

    // Signal headers must match
    REQUIRE(header2.m_signals.size() == header.m_signals.size());
    for (size_t i = 0; i < header.m_signals.size(); ++i) {
        auto const &orig = header.m_signals[i];
        auto const &copy = header2.m_signals[i];
        CHECK(copy.m_label == orig.m_label);
        CHECK(copy.m_samplesInDataRecord == orig.m_samplesInDataRecord);
        CHECK(copy.m_digitalMin == orig.m_digitalMin);
        CHECK(copy.m_digitalMax == orig.m_digitalMax);
        CHECK(copy.m_physicalMin == doctest::Approx(orig.m_physicalMin));
        CHECK(copy.m_physicalMax == doctest::Approx(orig.m_physicalMax));
    }

    // Verify sample data integrity: compare first few digital samples
    {
        std::ifstream origStream("test_generator_2.bdf", std::ios::binary);
        REQUIRE(origStream.is_open());
        auto origHeader = ReadHeaderExam(origStream);

        auto const &sig = origHeader.m_signals[0];
        auto origSamples = detail::CreateSignalSampleStore(
            origStream, origHeader.m_general, sig);

        auto const &sig2 = header2.m_signals[0];
        auto copySamples = detail::CreateSignalSampleStore(
            checkstream, header2.m_general, sig2);

        CHECK(copySamples.size() == origSamples.size());

        ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);

        auto limit = std::min(origSamples.size(),
                              static_cast<decltype(origSamples.size())>(50));
        auto origIt = origSamples.begin();
        auto copyIt = copySamples.begin();
        for (uint64_t i = 0; i < limit; ++i, ++origIt, ++copyIt) {
            int32_t origVal = proc(*origIt);
            int32_t copyVal = proc(*copyIt);
            CHECK(copyVal == origVal);
        }
    }

    checkstream.close();
    std::remove(tmpfile);
}
