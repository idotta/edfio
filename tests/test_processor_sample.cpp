#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <edfio/EdfIO.hpp>

using namespace edfio;

TEST_CASE("ProcessorSampleRecord sign-extends negative 2-byte samples") {
    // -100 as signed 16-bit = 0xFF9C
    Record<char> rec(2);
    rec()[0] = static_cast<char>(0xFF);
    rec()[1] = static_cast<char>(0x9C);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == -100);
}

TEST_CASE("ProcessorSampleRecord positive 2-byte samples") {
    Record<char> rec(2);
    rec()[0] = static_cast<char>(0x00);
    rec()[1] = static_cast<char>(0x64);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == 100);
}

TEST_CASE("ProcessorSampleRecord 3-byte BDF negative sample") {
    // -1000 as signed 24-bit = 0xFFFC18
    Record<char> rec(3);
    rec()[0] = static_cast<char>(0xFF);
    rec()[1] = static_cast<char>(0xFC);
    rec()[2] = static_cast<char>(0x18);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == -1000);
}

TEST_CASE("ProcessorSampleRecord 3-byte BDF positive sample") {
    // 1000 as signed 24-bit = 0x0003E8
    Record<char> rec(3);
    rec()[0] = static_cast<char>(0x00);
    rec()[1] = static_cast<char>(0x03);
    rec()[2] = static_cast<char>(0xE8);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == 1000);
}

TEST_CASE("ProcessorSampleRecord 2-byte max positive") {
    // 32767 = 0x7FFF
    Record<char> rec(2);
    rec()[0] = static_cast<char>(0x7F);
    rec()[1] = static_cast<char>(0xFF);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == 32767);
}

TEST_CASE("ProcessorSampleRecord 2-byte min negative") {
    // -32768 = 0x8000
    Record<char> rec(2);
    rec()[0] = static_cast<char>(0x80);
    rec()[1] = static_cast<char>(0x00);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == -32768);
}

TEST_CASE("ProcessorSampleRecord 2-byte minus one") {
    // -1 = 0xFFFF
    Record<char> rec(2);
    rec()[0] = static_cast<char>(0xFF);
    rec()[1] = static_cast<char>(0xFF);
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);
    auto result = proc(rec);
    CHECK(result == -1);
}
