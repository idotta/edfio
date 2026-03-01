#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>
#include <fstream>

using namespace edfio;

// ---- EDF+ format detection ----

TEST_CASE("test_generator_2.edf is EDF+ format") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    CHECK(IsEdf(header.m_general.m_version));
    CHECK(IsPlus(header.m_general.m_version));
    CHECK_FALSE(IsBdf(header.m_general.m_version));
    CHECK(header.m_general.m_version == DataFormat::EdfPlusC);
}

TEST_CASE("EDF+ has annotation channels") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    // Find annotation channels
    int annotCount = 0;
    int annotIdx = -1;
    for (size_t i = 0; i < header.m_signals.size(); ++i) {
        if (header.m_signals[i].m_detail.m_isAnnotation) {
            if (annotIdx < 0)
                annotIdx = static_cast<int>(i);
            ++annotCount;
        }
    }
    CHECK(annotCount > 0);
    REQUIRE(annotIdx >= 0);

    // Annotation channel label should contain "Annotation"
    CHECK(header.m_signals[annotIdx].m_label.find("Annotation") !=
          std::string::npos);
}

// ---- EDF+ header fields ----

TEST_CASE("EDF+ patient and recording fields are parsed") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    // EDF+ has structured patient/recording fields
    CHECK(header.m_general.m_totalSignals == 12);
    CHECK(header.m_general.m_datarecordsFile == 600);
    CHECK(header.m_general.m_datarecordDuration == doctest::Approx(1.0));
    CHECK(header.m_general.m_headerSize > 0);

    // Date should be 10-DEC-2009
    CHECK(header.m_general.m_startDate.day == 10);
    CHECK(header.m_general.m_startDate.month == 12);
    CHECK(header.m_general.m_startDate.year == 2009);
}

// ---- TimeStamp reading ----

TEST_CASE("EDF+ TimeStampStore reads timestamps from annotation channel") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    // Find first annotation signal
    const HeaderSignal *annotSignal = nullptr;
    for (auto const &sig : header.m_signals) {
        if (sig.m_detail.m_isAnnotation) {
            annotSignal = &sig;
            break;
        }
    }
    REQUIRE(annotSignal != nullptr);

    auto tsStore =
        detail::CreateTimeStampStore(stream, header.m_general, *annotSignal);
    CHECK(tsStore.size() == static_cast<uint64_t>(header.m_general.m_datarecordsFile));

    // Read first timestamp - TimeStampStore uses getline(delim=20) so the
    // result is the raw timestamp string (e.g. "+0"), NOT a full TAL record
    auto it = tsStore.begin();
    auto &rec = *it;
    CHECK(rec.Size() > 0);

    // The first byte should be '+' or '-'
    auto const &data = rec();
    CHECK((data[0] == '+' || data[0] == '-'));

    // Parse the timestamp value directly
    std::string tsStr(data.begin(), data.end());
    // Trim null bytes
    auto nullPos = tsStr.find('\0');
    if (nullPos != std::string::npos)
        tsStr.resize(nullPos);
    CHECK(!tsStr.empty());
    // First timestamp should start with '+'
    CHECK(tsStr[0] == '+');
}

TEST_CASE("EDF+ timestamps are monotonically non-decreasing") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    const HeaderSignal *annotSignal = nullptr;
    for (auto const &sig : header.m_signals) {
        if (sig.m_detail.m_isAnnotation) {
            annotSignal = &sig;
            break;
        }
    }
    REQUIRE(annotSignal != nullptr);

    auto tsStore =
        detail::CreateTimeStampStore(stream, header.m_general, *annotSignal);

    double prevStart = -1.0;
    int64_t count = 0;
    for (auto it = tsStore.begin(); it != tsStore.end() && count < 10; ++it, ++count) {
        auto &rec = *it;
        auto const &data = rec();
        // Extract timestamp string, trimming nulls
        std::string tsStr(data.begin(), data.end());
        auto nullPos = tsStr.find('\0');
        if (nullPos != std::string::npos)
            tsStr.resize(nullPos);
        REQUIRE(!tsStr.empty());
        REQUIRE((tsStr[0] == '+' || tsStr[0] == '-'));

        // Parse the numeric value
        double val = detail::ParseDouble(
            std::string_view(tsStr).substr(tsStr[0] == '+' ? 1 : 0),
            "bad timestamp");
        if (tsStr[0] == '-')
            val = -val;
        CHECK(val >= prevStart);
        prevStart = val;
    }
    CHECK(count == 10);
}

// ---- TAL (Time-stamped Annotation List) reading ----

TEST_CASE("EDF+ TAL reading via SignalRecordStore") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    const HeaderSignal *annotSignal = nullptr;
    for (auto const &sig : header.m_signals) {
        if (sig.m_detail.m_isAnnotation) {
            annotSignal = &sig;
            break;
        }
    }
    REQUIRE(annotSignal != nullptr);

    auto sigStore =
        detail::CreateSignalRecordStore(stream, header.m_general, *annotSignal);

    // Read first annotation record
    auto it = sigStore.begin();
    auto &rec = *it;
    CHECK(rec.Size() > 0);

    // The TAL data is the raw bytes of the annotation signal record
    // Convert to vector for ProcessTalRecord
    std::vector<char> talData(rec().begin(), rec().end());

    // First TAL should start with '+' or '-'
    REQUIRE(!talData.empty());
    CHECK((talData.front() == '+' || talData.front() == '-'));
}

TEST_CASE("EDF+ TalStore iterates through TALs") {
    std::ifstream stream("test_generator_2.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    const HeaderSignal *annotSignal = nullptr;
    for (auto const &sig : header.m_signals) {
        if (sig.m_detail.m_isAnnotation) {
            annotSignal = &sig;
            break;
        }
    }
    REQUIRE(annotSignal != nullptr);

    // Read first signal record to use as TalStore input
    auto sigStore =
        detail::CreateSignalRecordStore(stream, header.m_general, *annotSignal);
    auto sigIt = sigStore.begin();
    auto &rec = *sigIt;

    // Create TalStore from the record
    TalStore talStore(rec);
    int talCount = 0;
    for (auto it = talStore.begin(); it != talStore.end(); ++it) {
        auto &tal = *it;
        CHECK(!tal.empty());
        ++talCount;
    }
    // Should have at least the timestamp TAL
    CHECK(talCount >= 1);
}

TEST_CASE("ProcessTalRecord extracts annotations from TAL data") {
    // Build a synthetic TAL: "+0\x14\x14\x00" (timestamp only, no annotation text)
    std::vector<char> tal1 = {'+', '0', 20, 20, 0};
    auto annots1 = ProcessTalRecord(tal1, 0);
    // Timestamp-only TAL should produce no annotations (empty annotation text is skipped)
    CHECK(annots1.empty());

    // Build a TAL with an annotation: "+1.5\x14test annotation\x14\x00"
    std::vector<char> tal2;
    for (char c : std::string("+1.5"))
        tal2.push_back(c);
    tal2.push_back(20); // ANNOTATION_DIV
    for (char c : std::string("test annotation"))
        tal2.push_back(c);
    tal2.push_back(20); // ANNOTATION_DIV
    tal2.push_back(0);  // end

    auto annots2 = ProcessTalRecord(tal2, 5);
    REQUIRE(annots2.size() == 1);
    CHECK(annots2[0].m_start == doctest::Approx(1.5));
    CHECK(annots2[0].m_duration == doctest::Approx(0.0));
    CHECK(annots2[0].m_annotation == "test annotation");
    CHECK(annots2[0].m_datarecord == 5);
}

TEST_CASE("ProcessTalRecord with duration") {
    // "+2.0\x1530.5\x14my event\x14\x00"
    std::vector<char> tal;
    for (char c : std::string("+2.0"))
        tal.push_back(c);
    tal.push_back(21); // DURATION_DIV
    for (char c : std::string("30.5"))
        tal.push_back(c);
    tal.push_back(20); // ANNOTATION_DIV
    for (char c : std::string("my event"))
        tal.push_back(c);
    tal.push_back(20); // ANNOTATION_DIV
    tal.push_back(0);

    auto annots = ProcessTalRecord(tal, 0);
    REQUIRE(annots.size() == 1);
    CHECK(annots[0].m_start == doctest::Approx(2.0));
    CHECK(annots[0].m_duration == doctest::Approx(30.5));
    CHECK(annots[0].m_annotation == "my event");
}

TEST_CASE("ProcessTalRecord with multiple annotations in one TAL") {
    // "+0\x14event1\x14event2\x14\x00"
    std::vector<char> tal;
    for (char c : std::string("+0"))
        tal.push_back(c);
    tal.push_back(20); // ANNOTATION_DIV
    for (char c : std::string("event1"))
        tal.push_back(c);
    tal.push_back(20); // ANNOTATION_DIV
    for (char c : std::string("event2"))
        tal.push_back(c);
    tal.push_back(20); // ANNOTATION_DIV
    tal.push_back(0);

    auto annots = ProcessTalRecord(tal, 0);
    REQUIRE(annots.size() == 2);
    CHECK(annots[0].m_annotation == "event1");
    CHECK(annots[1].m_annotation == "event2");
    CHECK(annots[0].m_start == doctest::Approx(0.0));
    CHECK(annots[1].m_start == doctest::Approx(0.0));
}

TEST_CASE("ProcessTalRecord with negative onset") {
    std::vector<char> tal;
    for (char c : std::string("-5.25"))
        tal.push_back(c);
    tal.push_back(20);
    for (char c : std::string("neg event"))
        tal.push_back(c);
    tal.push_back(20);
    tal.push_back(0);

    auto annots = ProcessTalRecord(tal, 0);
    REQUIRE(annots.size() == 1);
    CHECK(annots[0].m_start == doctest::Approx(-5.25));
    CHECK(annots[0].m_annotation == "neg event");
}

TEST_CASE("ProcessTalRecord throws on invalid TAL") {
    // TAL not starting with '+' or '-'
    std::vector<char> bad = {'0', 20, 20, 0};
    CHECK_THROWS_AS(ProcessTalRecord(bad, 0), std::invalid_argument);
}

// ---- Annotation-only EDF+ file (Hypnogram) ----

TEST_CASE("Hypnogram EDF+ is annotation-only format") {
    std::ifstream stream("SC4001EC-Hypnogram.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    CHECK(IsEdf(header.m_general.m_version));
    CHECK(IsPlus(header.m_general.m_version));
    CHECK(header.m_general.m_version == DataFormat::EdfPlusC);

    // Should have exactly 1 signal (the annotation channel)
    CHECK(header.m_general.m_totalSignals == 1);
    CHECK(header.m_signals.size() == 1);
    CHECK(header.m_signals[0].m_detail.m_isAnnotation);
    CHECK(header.m_signals[0].m_label.find("Annotation") != std::string::npos);
}

TEST_CASE("Hypnogram annotations can be read") {
    std::ifstream stream("SC4001EC-Hypnogram.edf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);
    REQUIRE(header.m_signals.size() == 1);
    REQUIRE(header.m_signals[0].m_detail.m_isAnnotation);

    auto const &annotSignal = header.m_signals[0];
    auto sigStore =
        detail::CreateSignalRecordStore(stream, header.m_general, annotSignal);

    // Read annotation records and parse TALs
    int totalAnnotations = 0;
    int64_t drIdx = 0;
    for (auto it = sigStore.begin(); it != sigStore.end(); ++it, ++drIdx) {
        auto &rec = *it;
        std::vector<char> talData(rec().begin(), rec().end());
        if (talData.empty() || (talData.front() != '+' && talData.front() != '-'))
            continue;

        auto annots = ProcessTalRecord(talData, drIdx);
        totalAnnotations += static_cast<int>(annots.size());

        for (auto const &a : annots) {
            // Sleep stage annotations should have non-empty text
            CHECK(!a.m_annotation.empty());
            // Duration should be >= 0
            CHECK(a.m_duration >= 0.0);
        }
    }
    // Hypnogram should have many sleep stage annotations
    CHECK(totalAnnotations > 0);
}

// ---- ProcessTimeStamp (write) ----

TEST_CASE("ProcessTimeStamp creates correct record for positive time") {
    TimeStamp ts;
    ts.m_start = 5.0;
    ts.m_datarecord = 5;
    auto rec = ProcessTimeStamp(ts);
    std::string content(rec().begin(), rec().end());
    // Should start with +5
    CHECK(content[0] == '+');
    CHECK(content[1] == '5');
    // Should end with \x14\x14\x00
    auto sz = rec.Size();
    CHECK(rec()[sz - 3] == 20);
    CHECK(rec()[sz - 2] == 20);
    CHECK(rec()[sz - 1] == 0);
}

TEST_CASE("ProcessTimeStamp handles negative time") {
    TimeStamp ts;
    ts.m_start = -3.5;
    ts.m_datarecord = 0;
    auto rec = ProcessTimeStamp(ts);
    CHECK(rec()[0] == '-');
}

TEST_CASE("ProcessTimeStamp handles zero time") {
    TimeStamp ts;
    ts.m_start = 0.0;
    ts.m_datarecord = 0;
    auto rec = ProcessTimeStamp(ts);
    CHECK(rec()[0] == '+');
}

// ---- BDF+ format detection ----

TEST_CASE("test_generator_2.bdf is BDF+ format") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    CHECK(IsBdf(header.m_general.m_version));
    CHECK(IsPlus(header.m_general.m_version));
    CHECK_FALSE(IsEdf(header.m_general.m_version));
    CHECK(header.m_general.m_version == DataFormat::BdfPlusC);
}

TEST_CASE("BDF+ has annotation channels") {
    std::ifstream stream("test_generator_2.bdf", std::ios::binary);
    REQUIRE(stream.is_open());
    auto header = ReadHeaderExam(stream);

    int annotCount = 0;
    for (auto const &sig : header.m_signals) {
        if (sig.m_detail.m_isAnnotation)
            ++annotCount;
    }
    CHECK(annotCount > 0);
}
