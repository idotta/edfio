#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>
#include <cstdio>
#include <filesystem>
#include <fstream>

using namespace edfio;

// ===========================================================================
// 1. Open test_generator_2.edf -- format, signal count, duration
// ===========================================================================

TEST_CASE("EdfFile::open reads test_generator_2.edf successfully") {
    auto file = EdfFile::open("test_generator_2.edf");

    SUBCASE("format is EDF+") {
        CHECK(file.format() == DataFormat::EdfPlusC);
        CHECK(file.isEdf());
        CHECK(file.isPlus());
        CHECK_FALSE(file.isBdf());
    }

    SUBCASE("signal count matches header") {
        CHECK(file.signalCount() == 12);
        CHECK(file.signals().size() == 12);
    }

    SUBCASE("data record count and duration") {
        CHECK(file.dataRecordCount() == 600);
        // duration = dataRecordCount * datarecordDuration = 600 * 1.0 = 600
        CHECK(file.duration() == doctest::Approx(600.0));
    }

    SUBCASE("general header is accessible") {
        CHECK(file.general().m_headerSize > 0);
        CHECK(file.general().m_datarecordDuration == doctest::Approx(1.0));
        CHECK(file.general().m_startDate.day == 10);
        CHECK(file.general().m_startDate.month == 12);
        CHECK(file.general().m_startDate.year == 2009);
    }
}

// ===========================================================================
// 2. Read physical samples -- verify within physical range
// ===========================================================================

TEST_CASE("readSignal returns physical values within declared range") {
    auto file = EdfFile::open("test_generator_2.edf");

    // Find first non-annotation signal
    size_t sigIdx = 0;
    for (size_t i = 0; i < file.signals().size(); ++i) {
        if (!file.signals()[i].m_detail.m_isAnnotation) {
            sigIdx = i;
            break;
        }
    }
    auto const &sig = file.signals()[sigIdx];
    REQUIRE_FALSE(sig.m_detail.m_isAnnotation);

    auto samples = file.readSignal(sigIdx);
    REQUIRE_FALSE(samples.empty());

    // Expected sample count = dataRecordCount * samplesInDataRecord
    auto expectedCount = static_cast<size_t>(file.dataRecordCount()) *
                         static_cast<size_t>(sig.m_samplesInDataRecord);
    CHECK(samples.size() == expectedCount);

    // Verify first 200 samples are within declared physical range
    double physMin = sig.m_physicalMin;
    double physMax = sig.m_physicalMax;
    double tol = (physMax - physMin) * 0.001;
    auto limit = std::min(samples.size(), size_t{200});
    for (size_t i = 0; i < limit; ++i) {
        CHECK(samples[i] >= physMin - tol);
        CHECK(samples[i] <= physMax + tol);
    }
}

// ===========================================================================
// 3. Read digital samples -- verify within digital range
// ===========================================================================

TEST_CASE("readSignalDigital returns values within digital range") {
    auto file = EdfFile::open("test_generator_2.edf");

    // Find first non-annotation signal
    size_t sigIdx = 0;
    for (size_t i = 0; i < file.signals().size(); ++i) {
        if (!file.signals()[i].m_detail.m_isAnnotation) {
            sigIdx = i;
            break;
        }
    }
    auto const &sig = file.signals()[sigIdx];
    REQUIRE_FALSE(sig.m_detail.m_isAnnotation);

    auto samples = file.readSignalDigital(sigIdx);
    REQUIRE_FALSE(samples.empty());

    // EDF uses 16-bit signed integers
    int32_t digiMin = sig.m_digitalMin;
    int32_t digiMax = sig.m_digitalMax;

    auto limit = std::min(samples.size(), size_t{200});
    for (size_t i = 0; i < limit; ++i) {
        CHECK(samples[i] >= digiMin);
        CHECK(samples[i] <= digiMax);
    }
}

// ===========================================================================
// 4. Read annotations from EDF+ file
// ===========================================================================

TEST_CASE("readAnnotations extracts annotations from EDF+ file") {
    auto file = EdfFile::open("test_generator_2.edf");
    REQUIRE(file.isPlus());

    auto annots = file.readAnnotations();
    // EDF+ file should have at least some annotations
    CHECK(annots.size() > 0);

    for (auto const &a : annots) {
        CHECK_FALSE(a.m_annotation.empty());
        CHECK(a.m_duration >= 0.0);
    }
}

// ===========================================================================
// 5. Open test_generator_2.bdf -- verify BDF detection
// ===========================================================================

TEST_CASE("EdfFile::open reads BDF file correctly") {
    auto file = EdfFile::open("test_generator_2.bdf");

    CHECK(file.isBdf());
    CHECK(file.isPlus());
    CHECK_FALSE(file.isEdf());
    CHECK(file.format() == DataFormat::BdfPlusC);
    CHECK(file.signalCount() > 0);
    CHECK(file.dataRecordCount() > 0);
    CHECK(file.duration() > 0.0);

    SUBCASE("BDF physical samples within range") {
        // Find first non-annotation signal
        size_t sigIdx = 0;
        for (size_t i = 0; i < file.signals().size(); ++i) {
            if (!file.signals()[i].m_detail.m_isAnnotation) {
                sigIdx = i;
                break;
            }
        }
        auto const &sig = file.signals()[sigIdx];
        REQUIRE_FALSE(sig.m_detail.m_isAnnotation);

        auto samples = file.readSignal(sigIdx);
        REQUIRE_FALSE(samples.empty());

        double physMin = sig.m_physicalMin;
        double physMax = sig.m_physicalMax;
        double tol = (physMax - physMin) * 0.001;
        auto limit = std::min(samples.size(), size_t{100});
        for (size_t i = 0; i < limit; ++i) {
            CHECK(samples[i] >= physMin - tol);
            CHECK(samples[i] <= physMax + tol);
        }
    }

    SUBCASE("BDF digital samples within 24-bit range") {
        size_t sigIdx = 0;
        for (size_t i = 0; i < file.signals().size(); ++i) {
            if (!file.signals()[i].m_detail.m_isAnnotation) {
                sigIdx = i;
                break;
            }
        }
        auto const &sig = file.signals()[sigIdx];
        REQUIRE_FALSE(sig.m_detail.m_isAnnotation);

        auto samples = file.readSignalDigital(sigIdx);
        REQUIRE_FALSE(samples.empty());

        constexpr int32_t BDF_MIN = -8388608;
        constexpr int32_t BDF_MAX = 8388607;
        auto limit = std::min(samples.size(), size_t{100});
        for (size_t i = 0; i < limit; ++i) {
            CHECK(samples[i] >= BDF_MIN);
            CHECK(samples[i] <= BDF_MAX);
        }
    }
}

// ===========================================================================
// 6. Annotation-only EDF+ file (Hypnogram)
// ===========================================================================

TEST_CASE("EdfFile reads annotations from annotation-only file") {
    auto file = EdfFile::open("SC4001EC-Hypnogram.edf");

    CHECK(file.isEdf());
    CHECK(file.isPlus());
    CHECK(file.signalCount() == 1);
    CHECK(file.signals()[0].m_detail.m_isAnnotation);

    auto annots = file.readAnnotations();
    CHECK(annots.size() > 0);

    for (auto const &a : annots) {
        CHECK_FALSE(a.m_annotation.empty());
        CHECK(a.m_duration >= 0.0);
    }
}

// ===========================================================================
// 7. Invalid file path throws std::runtime_error
// ===========================================================================

TEST_CASE("EdfFile::open throws on non-existent file") {
    CHECK_THROWS_AS((void)EdfFile::open("no_such_file_xyz.edf"), std::runtime_error);
}

// ===========================================================================
// 8. Invalid signal index throws std::out_of_range
// ===========================================================================

TEST_CASE("EdfFile methods throw on invalid signal index") {
    auto file = EdfFile::open("test_generator_2.edf");
    auto badIdx = static_cast<size_t>(file.signalCount()) + 10;

    CHECK_THROWS_AS((void)file.readSignal(badIdx), std::out_of_range);
    CHECK_THROWS_AS((void)file.readSignalDigital(badIdx), std::out_of_range);
    CHECK_THROWS_AS((void)file.signalRecordStore(badIdx), std::out_of_range);
    CHECK_THROWS_AS((void)file.signalSampleStore(badIdx), std::out_of_range);
}

// ===========================================================================
// 9. EdfWriter round-trip
// ===========================================================================

TEST_CASE("EdfWriter round-trip preserves header and data") {
    const char *tmpfile = "test_edffile_roundtrip.edf";

    // Read the original file with EdfFile
    auto original = EdfFile::open("test_generator_2.edf");
    auto const &hdr = original.header();

    // Write using EdfWriter
    {
        auto writer = EdfWriter::create(tmpfile, hdr);
        auto store = original.dataRecordStore();
        for (auto it = store.begin(); it != store.end(); ++it) {
            writer.writeDataRecord(*it);
        }
        writer.close();
    }

    // Read back using EdfFile and verify
    {
        auto copy = EdfFile::open(tmpfile);

        CHECK(copy.format() == original.format());
        CHECK(copy.signalCount() == original.signalCount());
        CHECK(copy.dataRecordCount() == original.dataRecordCount());
        CHECK(copy.duration() == doctest::Approx(original.duration()));

        REQUIRE(copy.signals().size() == original.signals().size());
        for (size_t i = 0; i < copy.signals().size(); ++i) {
            CHECK(copy.signals()[i].m_label == original.signals()[i].m_label);
            CHECK(copy.signals()[i].m_samplesInDataRecord ==
                  original.signals()[i].m_samplesInDataRecord);
        }

        // Compare first non-annotation signal's digital samples
        size_t sigIdx = 0;
        for (size_t i = 0; i < copy.signals().size(); ++i) {
            if (!copy.signals()[i].m_detail.m_isAnnotation) {
                sigIdx = i;
                break;
            }
        }

        auto origSamples = original.readSignalDigital(sigIdx);
        auto copySamples = copy.readSignalDigital(sigIdx);
        REQUIRE(copySamples.size() == origSamples.size());

        auto limit = std::min(copySamples.size(), size_t{200});
        for (size_t i = 0; i < limit; ++i) {
            CHECK(copySamples[i] == origSamples[i]);
        }
    }

    std::remove(tmpfile);
}

// ===========================================================================
// 10. EdfWriter auto-patches data record count on close
// ===========================================================================

TEST_CASE("EdfWriter auto-patches data record count in header") {
    const char *tmpfile = "test_autocount.edf";

    auto original = EdfFile::open("test_generator_2.edf");
    auto hdr = original.header();

    // Deliberately set count to 0 -- the writer should fix it on close
    hdr.m_general.m_datarecordsFile = 0;

    {
        auto writer = EdfWriter::create(tmpfile, hdr);
        auto store = original.dataRecordStore();
        int64_t written = 0;
        for (auto it = store.begin(); it != store.end(); ++it) {
            writer.writeDataRecord(*it);
            ++written;
        }
        CHECK(writer.recordsWritten() == written);
        CHECK(written == 600);
        writer.close();
    }

    // Read back -- the header should now have the correct count
    {
        auto copy = EdfFile::open(tmpfile);
        CHECK(copy.dataRecordCount() == 600);
        CHECK(copy.duration() == doctest::Approx(600.0));

        // Verify data integrity too
        size_t sigIdx = 0;
        for (size_t i = 0; i < copy.signals().size(); ++i) {
            if (!copy.signals()[i].m_detail.m_isAnnotation) {
                sigIdx = i;
                break;
            }
        }
        auto origSamples = original.readSignalDigital(sigIdx);
        auto copySamples = copy.readSignalDigital(sigIdx);
        REQUIRE(copySamples.size() == origSamples.size());
        for (size_t i = 0; i < std::min(copySamples.size(), size_t{100}); ++i) {
            CHECK(copySamples[i] == origSamples[i]);
        }
    }

    std::remove(tmpfile);
}

TEST_CASE("EdfWriter destructor auto-patches count without explicit close") {
    const char *tmpfile = "test_autocount_dtor.edf";

    auto original = EdfFile::open("Calib5.edf");
    auto hdr = original.header();
    hdr.m_general.m_datarecordsFile = -1; // intentionally wrong

    {
        auto writer = EdfWriter::create(tmpfile, hdr);
        auto store = original.dataRecordStore();
        for (auto it = store.begin(); it != store.end(); ++it) {
            writer.writeDataRecord(*it);
        }
        // No explicit close() -- destructor should handle it
    }

    auto copy = EdfFile::open(tmpfile);
    CHECK(copy.dataRecordCount() == original.dataRecordCount());

    std::remove(tmpfile);
}

// ===========================================================================
// 11. EdfFile is moveable
// ===========================================================================

TEST_CASE("EdfFile supports move semantics") {
    auto file = EdfFile::open("test_generator_2.edf");
    auto moved = std::move(file);

    CHECK(moved.signalCount() == 12);
    CHECK(moved.format() == DataFormat::EdfPlusC);
}

// ===========================================================================
// 11. Low-level store access through EdfFile
// ===========================================================================

TEST_CASE("EdfFile provides low-level store access") {
    auto file = EdfFile::open("test_generator_2.edf");

    SUBCASE("dataRecordStore") {
        auto store = file.dataRecordStore();
        CHECK(store.size() ==
              static_cast<uint64_t>(file.dataRecordCount()));
    }

    SUBCASE("signalRecordStore") {
        size_t sigIdx = 0;
        auto store = file.signalRecordStore(sigIdx);
        CHECK(store.size() ==
              static_cast<uint64_t>(file.dataRecordCount()));
    }

    SUBCASE("signalSampleStore") {
        size_t sigIdx = 0;
        auto const &sig = file.signals()[sigIdx];
        auto store = file.signalSampleStore(sigIdx);
        auto expected = static_cast<uint64_t>(file.dataRecordCount()) *
                        static_cast<uint64_t>(sig.m_samplesInDataRecord);
        CHECK(store.size() == expected);
    }
}
