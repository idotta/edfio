#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>
#include <string>
#include <vector>

using namespace edfio;

// ---------------------------------------------------------------------------
// Helper: build a minimal HeaderSignal suitable for non-Plus EDF format
// ---------------------------------------------------------------------------
static HeaderSignal MakeEdfSignal(const std::string &label, double physMin,
                                  double physMax, int32_t digMin,
                                  int32_t digMax, int32_t samples) {
    HeaderSignal sig;
    sig.m_label = label;
    sig.m_transducer = "AgAgCl";
    sig.m_physDimension = "uV";
    sig.m_physicalMin = physMin;
    sig.m_physicalMax = physMax;
    sig.m_digitalMin = digMin;
    sig.m_digitalMax = digMax;
    sig.m_prefilter = "HP:0.1Hz";
    sig.m_samplesInDataRecord = samples;
    sig.m_reserved = "";
    return sig;
}

// ===========================================================================
// TEST SUITE: ProcessHeaderSignal / ProcessHeaderSignalFields round-trip
// ===========================================================================
TEST_SUITE("ProcessHeaderSignal round-trip") {

    TEST_CASE("EDF signal round-trips through fields and back") {
        // 1. Create a known HeaderSignal
        HeaderSignal original = MakeEdfSignal("EEG Fp1", -500.0, 500.0,
                                              -32768, 32767, 256);

        // 2. Convert to HeaderSignalFields
        std::vector<HeaderSignal> input{original};
        auto fields = ProcessHeaderSignal(std::move(input));

        REQUIRE(fields.size() == 1);

        // Verify intermediate field values are properly padded
        CHECK(fields[0].m_label().size() == 16);
        CHECK(fields[0].m_digitalMin().size() == 8);
        CHECK(fields[0].m_samplesInDataRecord().size() == 8);

        // 3. Convert back with EDF format (non-Plus, no annotation required)
        auto signals =
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0);

        REQUIRE(signals.size() == 1);
        const auto &out = signals[0];

        // Verify all fields round-trip correctly
        CHECK(out.m_label == "EEG Fp1");
        CHECK(out.m_transducer == "AgAgCl");
        CHECK(out.m_physDimension == "uV");
        CHECK(out.m_physicalMin == doctest::Approx(-500.0));
        CHECK(out.m_physicalMax == doctest::Approx(500.0));
        CHECK(out.m_digitalMin == -32768);
        CHECK(out.m_digitalMax == 32767);
        CHECK(out.m_prefilter == "HP:0.1Hz");
        CHECK(out.m_samplesInDataRecord == 256);

        // Verify computed detail fields
        double expectedScaling =
            (500.0 - (-500.0)) / (32767 - (-32768)); // 1000 / 65535
        CHECK(out.m_detail.m_scaling == doctest::Approx(expectedScaling));
        CHECK(out.m_detail.m_signalOffset == 0);
    }

    TEST_CASE("BDF signal round-trips with 24-bit digital range") {
        HeaderSignal original =
            MakeEdfSignal("EEG Fp2", -3200.0, 3200.0, -8388608, 8388607, 512);

        std::vector<HeaderSignal> input{original};
        auto fields = ProcessHeaderSignal(std::move(input));

        REQUIRE(fields.size() == 1);

        auto signals =
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Bdf, 1.0);

        REQUIRE(signals.size() == 1);
        const auto &out = signals[0];

        CHECK(out.m_digitalMin == -8388608);
        CHECK(out.m_digitalMax == 8388607);
        CHECK(out.m_physicalMin == doctest::Approx(-3200.0));
        CHECK(out.m_physicalMax == doctest::Approx(3200.0));
        CHECK(out.m_samplesInDataRecord == 512);

        // BDF uses 3 bytes per sample
        CHECK(out.m_detail.m_signalOffset == 0);
    }

    TEST_CASE("EDF signal with typical 16-bit range") {
        HeaderSignal original =
            MakeEdfSignal("EMG", -100.0, 100.0, -32768, 32767, 128);

        std::vector<HeaderSignal> input{original};
        auto fields = ProcessHeaderSignal(std::move(input));
        auto signals =
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0);

        REQUIRE(signals.size() == 1);
        const auto &out = signals[0];

        CHECK(out.m_digitalMin == -32768);
        CHECK(out.m_digitalMax == 32767);

        double expectedScaling =
            (100.0 - (-100.0)) / (32767 - (-32768)); // 200 / 65535
        CHECK(out.m_detail.m_scaling == doctest::Approx(expectedScaling));
    }

    TEST_CASE("Multiple signals: offset accumulates correctly for EDF") {
        HeaderSignal sig1 =
            MakeEdfSignal("EEG Fp1", -500.0, 500.0, -32768, 32767, 256);
        HeaderSignal sig2 =
            MakeEdfSignal("EEG Fp2", -500.0, 500.0, -32768, 32767, 128);

        std::vector<HeaderSignal> input{sig1, sig2};
        auto fields = ProcessHeaderSignal(std::move(input));
        auto signals =
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0);

        REQUIRE(signals.size() == 2);

        // First signal offset is 0
        CHECK(signals[0].m_detail.m_signalOffset == 0);
        // Second signal offset = first signal's samples * 2 bytes (EDF)
        CHECK(signals[1].m_detail.m_signalOffset == 256 * 2);
    }

    TEST_CASE("Multiple signals: offset accumulates correctly for BDF") {
        HeaderSignal sig1 =
            MakeEdfSignal("EEG Fp1", -500.0, 500.0, -8388608, 8388607, 256);
        HeaderSignal sig2 =
            MakeEdfSignal("EEG Fp2", -500.0, 500.0, -8388608, 8388607, 100);

        std::vector<HeaderSignal> input{sig1, sig2};
        auto fields = ProcessHeaderSignal(std::move(input));
        auto signals =
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Bdf, 1.0);

        REQUIRE(signals.size() == 2);

        CHECK(signals[0].m_detail.m_signalOffset == 0);
        // Second signal offset = first signal's samples * 3 bytes (BDF)
        CHECK(signals[1].m_detail.m_signalOffset == 256 * 3);
    }

} // TEST_SUITE "ProcessHeaderSignal round-trip"

// ===========================================================================
// TEST SUITE: ProcessHeaderSignalFields validation
// ===========================================================================
TEST_SUITE("ProcessHeaderSignalFields validation") {

    // Helper: create a valid single-signal HeaderSignalFields for non-Plus EDF
    static std::vector<HeaderSignalFields>
    MakeValidEdfFields(int32_t digMin = -32768, int32_t digMax = 32767,
                       int32_t samples = 256) {
        HeaderSignalFields f;
        f.m_label("EEG Fp1");
        f.m_transducer("AgAgCl");
        f.m_physDimension("uV");
        f.m_physicalMin("-500");
        f.m_physicalMax("500");
        f.m_digitalMin(std::to_string(digMin));
        f.m_digitalMax(std::to_string(digMax));
        f.m_prefilter("HP:0.1Hz");
        f.m_samplesInDataRecord(std::to_string(samples));
        f.m_reserved("");
        return {f};
    }

    TEST_CASE("digitalMax equal to digitalMin throws (div-by-zero)") {
        auto fields = MakeValidEdfFields(100, 100, 256);
        // digitalMax < digitalMin + 1 means digitalMax == digitalMin fails
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("digitalMin greater than digitalMax throws") {
        auto fields = MakeValidEdfFields(1000, -1000, 256);
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("samplesInDataRecord less than 1 throws") {
        auto fields = MakeValidEdfFields(-32768, 32767, 0);
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("samplesInDataRecord negative throws") {
        auto fields = MakeValidEdfFields(-32768, 32767, -1);
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("EDF digital min out of range throws") {
        // digitalMin = -32769, below EDF 16-bit signed range
        auto fields = MakeValidEdfFields(-32769, 32767, 256);
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("EDF digital max out of range throws") {
        // digitalMax = 32768, above EDF 16-bit signed range
        auto fields = MakeValidEdfFields(-32768, 32768, 256);
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("BDF digital min out of range throws") {
        // -8388609 is below BDF 24-bit signed range
        // Use BDF fields helper
        HeaderSignalFields f;
        f.m_label("EEG Fp1");
        f.m_transducer("AgAgCl");
        f.m_physDimension("uV");
        f.m_physicalMin("-500");
        f.m_physicalMax("500");
        f.m_digitalMin("-8388609");
        f.m_digitalMax("8388607");
        f.m_prefilter("HP:0.1Hz");
        f.m_samplesInDataRecord("256");
        f.m_reserved("");
        std::vector<HeaderSignalFields> fields{f};
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Bdf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("BDF digital max out of range throws") {
        HeaderSignalFields f;
        f.m_label("EEG Fp1");
        f.m_transducer("AgAgCl");
        f.m_physDimension("uV");
        f.m_physicalMin("-500");
        f.m_physicalMax("500");
        f.m_digitalMin("-8388608");
        f.m_digitalMax("8388608");
        f.m_prefilter("HP:0.1Hz");
        f.m_samplesInDataRecord("256");
        f.m_reserved("");
        std::vector<HeaderSignalFields> fields{f};
        CHECK_THROWS_AS(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Bdf, 1.0),
            std::invalid_argument);
    }

    TEST_CASE("Valid EDF boundary values do not throw") {
        auto fields = MakeValidEdfFields(-32768, 32767, 1);
        CHECK_NOTHROW(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Edf, 1.0));
    }

    TEST_CASE("Valid BDF boundary values do not throw") {
        HeaderSignalFields f;
        f.m_label("EEG Fp1");
        f.m_transducer("AgAgCl");
        f.m_physDimension("uV");
        f.m_physicalMin("-500");
        f.m_physicalMax("500");
        f.m_digitalMin("-8388608");
        f.m_digitalMax("8388607");
        f.m_prefilter("HP:0.1Hz");
        f.m_samplesInDataRecord("256");
        f.m_reserved("");
        std::vector<HeaderSignalFields> fields{f};
        CHECK_NOTHROW(
            ProcessHeaderSignalFields(std::move(fields), DataFormat::Bdf, 1.0));
    }

} // TEST_SUITE "ProcessHeaderSignalFields validation"

// ===========================================================================
// TEST SUITE: ProcessAnnotation
// ===========================================================================
TEST_SUITE("ProcessAnnotation") {

    TEST_CASE("Positive start, no duration") {
        Annotation ann;
        ann.m_start = 1.5;
        ann.m_duration = 0;
        ann.m_annotation = "TestEvent";

        auto record = ProcessAnnotation(ann);
        const auto &bytes = record();

        // Expected layout: "+1.5" \x14 "TestEvent" \x14 \x00
        std::string content(bytes.begin(), bytes.end());

        // Verify timestamp starts with '+'
        CHECK(content[0] == '+');
        // Verify timestamp value is present
        CHECK(content.find("+1.5") == 0);

        // Verify ANNOTATION_DIV (0x14 = 20) separates timestamp from annotation
        size_t firstDiv = content.find(static_cast<char>(20));
        REQUIRE(firstDiv != std::string::npos);

        // No duration means no DURATION_DIV (21) before the ANNOTATION_DIV
        for (size_t i = 0; i < firstDiv; ++i) {
            CHECK(content[i] != static_cast<char>(21));
        }

        // Verify annotation text follows the first div
        std::string annotText =
            content.substr(firstDiv + 1, std::string("TestEvent").size());
        CHECK(annotText == "TestEvent");

        // Verify record ends with \x14 \x00
        CHECK(bytes[bytes.size() - 2] == static_cast<char>(20));
        CHECK(bytes[bytes.size() - 1] == static_cast<char>(0));
    }

    TEST_CASE("Negative start produces '-' prefix") {
        Annotation ann;
        ann.m_start = -2.0;
        ann.m_duration = 0;
        ann.m_annotation = "Onset";

        auto record = ProcessAnnotation(ann);
        const auto &bytes = record();
        std::string content(bytes.begin(), bytes.end());

        // Negative start should begin with '-', not '+'
        CHECK(content[0] == '-');
        CHECK(content.find("-2") == 0);
    }

    TEST_CASE("Positive duration includes duration field") {
        Annotation ann;
        ann.m_start = 10.0;
        ann.m_duration = 2.5;
        ann.m_annotation = "Stimulus";

        auto record = ProcessAnnotation(ann);
        const auto &bytes = record();
        std::string content(bytes.begin(), bytes.end());

        // Layout: "+10" \x15 "2.5" \x14 "Stimulus" \x14 \x00
        // DURATION_DIV (21 = 0x15) must be present
        size_t durDiv = content.find(static_cast<char>(21));
        REQUIRE(durDiv != std::string::npos);

        // Duration text follows the DURATION_DIV
        size_t annotDiv = content.find(static_cast<char>(20));
        std::string durText = content.substr(durDiv + 1, annotDiv - durDiv - 1);
        CHECK(durText == "2.5");
    }

    TEST_CASE("Zero duration omits duration field") {
        Annotation ann;
        ann.m_start = 5.0;
        ann.m_duration = 0.0;
        ann.m_annotation = "Marker";

        auto record = ProcessAnnotation(ann);
        const auto &bytes = record();
        std::string content(bytes.begin(), bytes.end());

        // No DURATION_DIV (21) should appear before the first ANNOTATION_DIV (20)
        size_t annotDiv = content.find(static_cast<char>(20));
        REQUIRE(annotDiv != std::string::npos);
        for (size_t i = 0; i < annotDiv; ++i) {
            CHECK(content[i] != static_cast<char>(21));
        }
    }

    TEST_CASE("Empty annotation string throws") {
        Annotation ann;
        ann.m_start = 0.0;
        ann.m_duration = 0.0;
        ann.m_annotation = "";

        CHECK_THROWS_AS(ProcessAnnotation(ann), std::invalid_argument);
    }

    TEST_CASE("Zero start gets '+' prefix") {
        Annotation ann;
        ann.m_start = 0.0;
        ann.m_duration = 0.0;
        ann.m_annotation = "Start";

        auto record = ProcessAnnotation(ann);
        const auto &bytes = record();

        // 0.0 >= 0, so '+' prefix
        CHECK(bytes[0] == '+');
    }

    TEST_CASE("Annotation record has correct total size") {
        Annotation ann;
        ann.m_start = 3.0;
        ann.m_duration = 1.5;
        ann.m_annotation = "Event";

        auto record = ProcessAnnotation(ann);

        // Expected: "+3" (2) + \x15 (1) + "1.5" (3) + \x14 (1) + "Event" (5) +
        //           \x14 (1) + \x00 (1) = 14
        CHECK(record.Size() == 14);
    }

} // TEST_SUITE "ProcessAnnotation"

// ===========================================================================
// TEST SUITE: Sample conversion round-trip
// ===========================================================================
TEST_SUITE("Sample conversion round-trip") {

    TEST_CASE("Digital 2-byte round-trip: encode then decode") {
        // Use identity conversion (offset=0, scaling=1)
        const double offset = 0.0;
        const double scaling = 1.0;
        const uint32_t sampleSize = 2; // EDF

        ProcessorSample<SampleType::Digital> encoder(offset, scaling,
                                                     sampleSize);
        ProcessorSampleRecord<SampleType::Digital> decoder(offset, scaling);

        SUBCASE("Positive value") {
            int32_t original = 1234;
            auto record = encoder(original);
            REQUIRE(record.Size() == 2);
            int32_t decoded = decoder(record);
            CHECK(decoded == original);
        }

        SUBCASE("Negative value") {
            int32_t original = -1234;
            auto record = encoder(original);
            REQUIRE(record.Size() == 2);
            int32_t decoded = decoder(record);
            CHECK(decoded == original);
        }

        SUBCASE("Zero value") {
            int32_t original = 0;
            auto record = encoder(original);
            REQUIRE(record.Size() == 2);
            int32_t decoded = decoder(record);
            CHECK(decoded == original);
        }
    }

    TEST_CASE("Digital 3-byte (BDF) round-trip: encode then decode") {
        const double offset = 0.0;
        const double scaling = 1.0;
        const uint32_t sampleSize = 3; // BDF

        ProcessorSample<SampleType::Digital> encoder(offset, scaling,
                                                     sampleSize);
        ProcessorSampleRecord<SampleType::Digital> decoder(offset, scaling);

        SUBCASE("Positive value") {
            int32_t original = 100000;
            auto record = encoder(original);
            REQUIRE(record.Size() == 3);
            int32_t decoded = decoder(record);
            CHECK(decoded == original);
        }

        SUBCASE("Negative value") {
            int32_t original = -100000;
            auto record = encoder(original);
            REQUIRE(record.Size() == 3);
            int32_t decoded = decoder(record);
            CHECK(decoded == original);
        }

        SUBCASE("Zero value") {
            int32_t original = 0;
            auto record = encoder(original);
            REQUIRE(record.Size() == 3);
            int32_t decoded = decoder(record);
            CHECK(decoded == original);
        }
    }

    TEST_CASE("Physical to digital round-trip through records") {
        // Simulate a real EDF signal: physMin=-500, physMax=500,
        // digMin=-32768, digMax=32767
        const double physMin = -500.0;
        const double physMax = 500.0;
        const int32_t digMin = -32768;
        const int32_t digMax = 32767;
        const double scaling = (physMax - physMin) / (digMax - digMin);
        const double offset = physMin - scaling * digMin;
        const uint32_t sampleSize = 2;

        // Physical -> Digital -> Record -> Digital -> Physical
        double physValue = 123.456;
        int32_t digital = ConvertSample(offset, scaling, physValue);

        ProcessorSample<SampleType::Digital> encoder(offset, scaling,
                                                     sampleSize);
        ProcessorSampleRecord<SampleType::Digital> decoder(offset, scaling);

        auto record = encoder(digital);
        int32_t decodedDigital = decoder(record);

        // Digital value must survive the record round-trip exactly
        CHECK(decodedDigital == digital);

        // Convert back to physical
        double decodedPhysical = ConvertSample(offset, scaling, decodedDigital);
        // Allow small error due to quantization
        CHECK(decodedPhysical == doctest::Approx(physValue).epsilon(0.001));
    }

    TEST_CASE("2-byte boundary values round-trip") {
        const double offset = 0.0;
        const double scaling = 1.0;
        const uint32_t sampleSize = 2;

        ProcessorSample<SampleType::Digital> encoder(offset, scaling,
                                                     sampleSize);
        ProcessorSampleRecord<SampleType::Digital> decoder(offset, scaling);

        SUBCASE("Max positive: 32767") {
            int32_t original = 32767;
            auto record = encoder(original);
            int32_t decoded = decoder(record);
            CHECK(decoded == 32767);
        }

        SUBCASE("Min negative: -32768") {
            int32_t original = -32768;
            auto record = encoder(original);
            int32_t decoded = decoder(record);
            CHECK(decoded == -32768);
        }

        SUBCASE("Minus one: -1") {
            int32_t original = -1;
            auto record = encoder(original);
            int32_t decoded = decoder(record);
            CHECK(decoded == -1);
        }

        SUBCASE("Plus one: 1") {
            int32_t original = 1;
            auto record = encoder(original);
            int32_t decoded = decoder(record);
            CHECK(decoded == 1);
        }
    }

    TEST_CASE("3-byte boundary values round-trip") {
        const double offset = 0.0;
        const double scaling = 1.0;
        const uint32_t sampleSize = 3;

        ProcessorSample<SampleType::Digital> encoder(offset, scaling,
                                                     sampleSize);
        ProcessorSampleRecord<SampleType::Digital> decoder(offset, scaling);

        SUBCASE("Max positive: 8388607") {
            int32_t original = 8388607;
            auto record = encoder(original);
            int32_t decoded = decoder(record);
            CHECK(decoded == 8388607);
        }

        SUBCASE("Min negative: -8388608") {
            int32_t original = -8388608;
            auto record = encoder(original);
            int32_t decoded = decoder(record);
            CHECK(decoded == -8388608);
        }

        SUBCASE("Minus one: -1") {
            int32_t original = -1;
            auto record = encoder(original);
            int32_t decoded = decoder(record);
            CHECK(decoded == -1);
        }
    }

    TEST_CASE("ConvertSample digital-to-physical and back") {
        const double physMin = -3200.0;
        const double physMax = 3200.0;
        const int32_t digMin = -8388608;
        const int32_t digMax = 8388607;
        const double scaling = (physMax - physMin) / (digMax - digMin);
        const double offset = physMin - scaling * digMin;

        SUBCASE("Mid-range digital value") {
            int32_t digital = 0;
            double physical = ConvertSample(offset, scaling, digital);
            int32_t backDigital = ConvertSample(offset, scaling, physical);
            CHECK(backDigital == digital);
        }

        SUBCASE("Max digital value") {
            int32_t digital = digMax;
            double physical = ConvertSample(offset, scaling, digital);
            CHECK(physical == doctest::Approx(physMax).epsilon(0.001));
        }

        SUBCASE("Min digital value") {
            int32_t digital = digMin;
            double physical = ConvertSample(offset, scaling, digital);
            CHECK(physical == doctest::Approx(physMin).epsilon(0.001));
        }
    }

    TEST_CASE("Physical round-trip via ProcessorSample<Physical>") {
        // Use Physical sample type which internally converts phys->dig->record
        const double physMin = -500.0;
        const double physMax = 500.0;
        const int32_t digMin = -32768;
        const int32_t digMax = 32767;
        const double scaling = (physMax - physMin) / (digMax - digMin);
        const double offset = physMin - scaling * digMin;
        const uint32_t sampleSize = 2;

        ProcessorSample<SampleType::Physical> encoder(offset, scaling,
                                                      sampleSize);
        ProcessorSampleRecord<SampleType::Physical> decoder(offset, scaling);

        SUBCASE("Positive physical value") {
            double original = 250.0;
            auto record = encoder(original);
            double decoded = decoder(record);
            CHECK(decoded == doctest::Approx(original).epsilon(0.1));
        }

        SUBCASE("Negative physical value") {
            double original = -250.0;
            auto record = encoder(original);
            double decoded = decoder(record);
            CHECK(decoded == doctest::Approx(original).epsilon(0.1));
        }

        SUBCASE("Zero physical value") {
            double original = 0.0;
            auto record = encoder(original);
            double decoded = decoder(record);
            CHECK(decoded == doctest::Approx(original).epsilon(0.1));
        }
    }

} // TEST_SUITE "Sample conversion round-trip"

// ===========================================================================
// TEST SUITE: detail::to_string_decimal
// ===========================================================================
TEST_SUITE("to_string_decimal") {

    TEST_CASE("Integer values have no trailing zeros or decimal point") {
        // 5.0 should become "5", not "5.000000"
        auto result = detail::to_string_decimal(5.0);
        CHECK(result == "5");

        result = detail::to_string_decimal(100.0);
        CHECK(result == "100");

        result = detail::to_string_decimal(0.0);
        CHECK(result == "0");
    }

    TEST_CASE("Decimal values have trailing zeros trimmed") {
        // 2.5 should become "2.5", not "2.500000"
        auto result = detail::to_string_decimal(2.5);
        CHECK(result == "2.5");

        result = detail::to_string_decimal(1.25);
        CHECK(result == "1.25");

        result = detail::to_string_decimal(3.1);
        CHECK(result == "3.1");
    }

    TEST_CASE("Negative integer values") {
        auto result = detail::to_string_decimal(-10.0);
        CHECK(result == "-10");

        result = detail::to_string_decimal(-1.0);
        CHECK(result == "-1");
    }

    TEST_CASE("Negative decimal values") {
        auto result = detail::to_string_decimal(-2.5);
        CHECK(result == "-2.5");

        result = detail::to_string_decimal(-0.125);
        CHECK(result == "-0.125");
    }

    TEST_CASE("Small fractional values") {
        auto result = detail::to_string_decimal(0.5);
        CHECK(result == "0.5");

        result = detail::to_string_decimal(0.1);
        // std::to_string(0.1) gives "0.100000", trimmed to "0.1"
        CHECK(result == "0.1");
    }

    TEST_CASE("Large integer values") {
        auto result = detail::to_string_decimal(32767.0);
        CHECK(result == "32767");

        result = detail::to_string_decimal(-32768.0);
        CHECK(result == "-32768");
    }

} // TEST_SUITE "to_string_decimal"
