# edfio

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![Header-only](https://img.shields.io/badge/header--only-yes-green.svg)]()

A modern C++23 header-only library for reading and writing
[EDF](https://www.edfplus.info/specs/edf.html) (European Data Format) and
[BDF](https://www.biosemi.com/faq/file_format.htm) (BioSemi Data Format)
biomedical signal files.

## Features

- **Header-only** -- zero external dependencies, single `#include` to get started
- **Modern C++23** -- uses `std::span`, `std::ranges`, `constexpr`, `std::from_chars`
- **Iterator-based** random access to data records and individual signal samples
- **Full format coverage** -- EDF, EDF+C, EDF+D, BDF, BDF+C, BDF+D
- **EDF+/BDF+ annotations** -- read and write Time-stamped Annotation Lists (TALs)
- **Two API layers** -- high-level `EdfFile` for common tasks, low-level stores and sinks for full control

## Requirements

| Dependency | Minimum version |
|---|---|
| C++ standard | C++23 |
| Clang | 17+ |
| GCC | 13+ |
| MSVC | 19.36+ (Visual Studio 2022 17.6) |
| CMake | 4.0+ |

## Quick Start

### High-level API

```cpp
#include <edfio/EdfFile.hpp>
#include <iostream>

int main() {
    auto file = edfio::EdfFile::open("recording.edf");

    std::cout << "Format: " << (file.isEdf() ? "EDF" : "BDF") << "\n";
    std::cout << "Signals: " << file.signalCount() << "\n";
    std::cout << "Data records: " << file.dataRecordCount() << "\n";
    std::cout << "Duration: " << file.duration() << "s\n";

    // Access header information
    auto const& general = file.general();
    auto const& signals = file.signals();

    // Read physical (scaled) samples for the first signal
    auto samples = file.readSignal(0);

    // Read raw digital samples
    auto digital = file.readSignalDigital(0);

    // Read annotations (EDF+/BDF+ only)
    if (file.isPlus()) {
        auto annotations = file.readAnnotations();
        for (auto const& annot : annotations) {
            std::cout << annot.m_start << "s: "
                      << annot.m_annotation << "\n";
        }
    }
}
```

### Writing with the high-level API

```cpp
#include <edfio/EdfFile.hpp>

// Prepare a header from an existing file, or build one manually
auto source = edfio::EdfFile::open("source.edf");
auto writer = edfio::EdfWriter::create("output.edf", source.general(), source.signals());

// Write data records
// ...

writer.close();
```

## Installation

### CMake FetchContent (recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
    edfio
    GIT_REPOSITORY https://github.com/idotta/edfio.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(edfio)

target_link_libraries(your_target PRIVATE edfio)
```

### CMake add\_subdirectory

Clone or add edfio as a git submodule, then:

```cmake
add_subdirectory(external/edfio)
target_link_libraries(your_target PRIVATE edfio)
```

### Copy headers

Copy the `include/edfio/` directory into your project's include path and ensure
your build system enables C++23 (`-std=c++23` or `cxx_std_23`).

## Low-level API

The low-level API provides direct iterator access to data records, signal
records, and individual samples through **stores** (reading) and **sinks**
(writing).

### Reading a file

```cpp
#include <edfio/EdfIO.hpp>
#include <fstream>
#include <iostream>

int main() {
    std::ifstream stream("recording.edf", std::ios::binary);

    // Parse the full header (general + all signal headers)
    auto header = edfio::ReadHeaderExam(stream);
    auto const& general = header.m_general;
    auto const& signals = header.m_signals;

    std::cout << "Format: "
              << (edfio::IsEdf(general.m_version) ? "EDF" : "BDF")
              << (edfio::IsPlus(general.m_version) ? "+" : "") << "\n";
    std::cout << "Signals: " << general.m_totalSignals << "\n";
    std::cout << "Data records: " << general.m_datarecordsFile << "\n";
    std::cout << "Record duration: " << general.m_datarecordDuration << "s\n";

    // Print signal labels
    for (auto const& sig : signals) {
        std::cout << "  " << sig.m_label
                  << " [" << sig.m_physDimension << "]"
                  << " " << sig.m_samplesInDataRecord << " samples/record\n";
    }

    // Iterate over raw data records
    auto store = edfio::detail::CreateDataRecordStore(stream, general);
    for (auto it = store.begin(); it != store.end(); ++it) {
        auto const& record = *it;
        // record() returns std::vector<char> with the raw bytes
    }
}
```

### Reading signal samples

```cpp
#include <edfio/EdfIO.hpp>
#include <fstream>
#include <vector>

int main() {
    std::ifstream stream("recording.edf", std::ios::binary);
    auto header = edfio::ReadHeaderExam(stream);
    auto const& general = header.m_general;
    auto const& signal = header.m_signals[0]; // first signal

    // Create a sample store -- iterates individual samples across all
    // data records for a single signal
    auto sampleStore = edfio::detail::CreateSignalSampleStore(
        stream, general, signal);

    // Processor converts raw bytes to physical (double) or digital (int32_t) values
    edfio::ProcessorSampleRecord<edfio::SampleType::Physical> toPhysical(
        signal.m_detail.m_offset,
        signal.m_detail.m_scaling);

    std::vector<double> samples;
    samples.reserve(sampleStore.size());
    for (auto it = sampleStore.begin(); it != sampleStore.end(); ++it) {
        samples.push_back(toPhysical(*it));
    }

    // For digital (raw integer) values, use SampleType::Digital instead:
    edfio::ProcessorSampleRecord<edfio::SampleType::Digital> toDigital(
        signal.m_detail.m_offset,
        signal.m_detail.m_scaling);
}
```

### Reading signal records per data record

```cpp
// Read signal-level records (one per data record) for a specific signal
auto sigStore = edfio::detail::CreateSignalRecordStore(
    stream, header.m_general, header.m_signals[0]);

for (auto it = sigStore.begin(); it != sigStore.end(); ++it) {
    auto const& record = *it;
    // record() contains one data record's worth of samples for this signal
}
```

## Writing Files

```cpp
#include <edfio/EdfIO.hpp>
#include <fstream>

int main() {
    // Read an existing file
    std::ifstream inStream("source.edf", std::ios::binary);
    auto header = edfio::ReadHeaderExam(inStream);

    // Write header to a new file
    std::ofstream outStream("copy.edf", std::ios::binary);
    edfio::WriteHeaderExam(outStream, header);

    // Copy data records using store (read) and sink (write) iterators
    auto store = edfio::detail::CreateDataRecordStore(inStream, header.m_general);
    auto sink = edfio::detail::CreateDataRecordSink(outStream, header.m_general);
    auto sinkIt = sink.begin();
    for (auto it = store.begin(); it != store.end(); ++it) {
        *sinkIt = *it;
        ++sinkIt;
    }
}
```

### Building a header from scratch

```cpp
#include <edfio/EdfIO.hpp>
#include <edfio/header/HeaderUtils.hpp>
#include <fstream>
#include <vector>

int main() {
    // Define signals
    std::vector<edfio::HeaderSignal> signals;
    signals.push_back(edfio::detail::CreateHeaderSignal(
        "EEG Fp1",        // label
        256,              // samples per data record
        -3200.0,          // physical min
        3200.0,           // physical max
        -32768,           // digital min
        32767             // digital max
    ));

    // Calculate header size: 256 bytes (general) + 256 bytes per signal
    int32_t headerSize = 256 + 256 * static_cast<int32_t>(signals.size());

    // Create the general header
    auto general = edfio::detail::CreateHeaderGeneral(
        edfio::DataFormat::Edf,
        "Patient X",      // patient
        "Recording 1",    // recording
        1, 3, 2026,       // start date (day, month, year)
        14, 30, 0,        // start time (hour, minute, second)
        headerSize,
        "",               // reserved
        100,              // number of data records
        1.0,              // data record duration in seconds
        signals
    );

    // Assemble and write
    edfio::HeaderExam exam{general, signals};
    std::ofstream outStream("new_file.edf", std::ios::binary);
    edfio::WriteHeaderExam(outStream, exam);

    // Write sample data using ProcessorSample and DataRecordSink...
}
```

## EDF+ / BDF+ Annotations

EDF+ and BDF+ files store annotations in dedicated signal channels using
Time-stamped Annotation Lists (TALs). The `Annotation` struct holds the onset
time, duration, and annotation text:

```cpp
struct Annotation : TimeStamp {
    double m_duration;
    std::string m_annotation;
    // inherited: double m_start, int64_t m_datarecord
};
```

### Reading annotations

```cpp
#include <edfio/EdfIO.hpp>
#include <fstream>
#include <iostream>

int main() {
    std::ifstream stream("recording_plus.edf", std::ios::binary);
    auto header = edfio::ReadHeaderExam(stream);

    if (!edfio::IsPlus(header.m_general.m_version)) {
        std::cout << "Not an EDF+/BDF+ file\n";
        return 0;
    }

    // Find the annotation signal
    edfio::HeaderSignal const* annotSignal = nullptr;
    for (auto const& sig : header.m_signals) {
        if (sig.m_detail.m_isAnnotation) {
            annotSignal = &sig;
            break;
        }
    }

    // Read annotation records and parse TALs
    auto sigStore = edfio::detail::CreateSignalRecordStore(
        stream, header.m_general, *annotSignal);

    int64_t drIdx = 0;
    for (auto it = sigStore.begin(); it != sigStore.end(); ++it, ++drIdx) {
        auto const& rec = *it;
        std::vector<char> talData(rec().begin(), rec().end());

        auto annotations = edfio::ProcessTalRecord(talData, drIdx);
        for (auto const& annot : annotations) {
            std::cout << "  [" << annot.m_start << "s";
            if (annot.m_duration > 0)
                std::cout << ", dur=" << annot.m_duration << "s";
            std::cout << "] " << annot.m_annotation << "\n";
        }
    }
}
```

### Writing annotations

```cpp
#include <edfio/EdfIO.hpp>

// Create an annotation
edfio::Annotation annot;
annot.m_start = 1.5;
annot.m_duration = 30.0;
annot.m_annotation = "Sleep Stage W";
annot.m_datarecord = 0;

// Serialize to a TAL record
auto record = edfio::ProcessAnnotation(annot);

// Create a timestamp record for the data record onset
edfio::TimeStamp ts;
ts.m_start = 0.0;
ts.m_datarecord = 0;
auto tsRecord = edfio::ProcessTimeStamp(ts);

// Combine timestamp + annotation into the annotation signal's data
auto combined = tsRecord + record;
```

### Reading timestamps

EDF+ files include a timestamp at the start of each data record in the
annotation channel. Use `TimeStampStore` to access them:

```cpp
auto tsStore = edfio::detail::CreateTimeStampStore(
    stream, header.m_general, *annotSignal);

for (auto it = tsStore.begin(); it != tsStore.end(); ++it) {
    auto const& rec = *it;
    // Parse with ProcessTimeStampRecord for structured TimeStamp data
    auto ts = edfio::ProcessTimeStampRecord(*it, /* datarecord index */);
}
```

## Data Formats

edfio detects and supports all standard EDF/BDF variants through the
`DataFormat` enum:

| Enum value | Description | Sample size |
|---|---|---|
| `DataFormat::Edf` | Standard EDF | 2 bytes (16-bit) |
| `DataFormat::EdfPlusC` | EDF+ Continuous | 2 bytes |
| `DataFormat::EdfPlusD` | EDF+ Discontinuous | 2 bytes |
| `DataFormat::Bdf` | Standard BDF | 3 bytes (24-bit) |
| `DataFormat::BdfPlusC` | BDF+ Continuous | 3 bytes |
| `DataFormat::BdfPlusD` | BDF+ Discontinuous | 3 bytes |

Helper functions: `IsEdf()`, `IsBdf()`, `IsPlus()`, `GetSampleBytes()`.

## Building and Testing

```bash
cmake -B build -G Ninja
cmake --build build
ctest --test-dir build
```

The test suite uses [doctest](https://github.com/doctest/doctest) (bundled in
`third_party/`) and runs against sample EDF/EDF+/BDF+ files in the `files/`
directory.

## Project Structure

```
edfio/
  CMakeLists.txt
  LICENSE
  include/edfio/
    EdfIO.hpp                         # Main include (low-level API)
    EdfFile.hpp                       # High-level API (coming soon)
    core/
      Annotation.hpp                  # Annotation and TimeStamp structs
      DataFormat.hpp                  # DataFormat enum and helpers
      Record.hpp                      # Record<T> container with stream I/O
      SampleType.hpp                  # SampleType enum, Sample traits, ConvertSample
      Field.hpp                       # Fixed-size header field type
      StreamIO.hpp                    # Reader/Writer stream type aliases
    header/
      HeaderExam.hpp                  # HeaderExam (general + signal headers)
      HeaderGeneral.hpp               # HeaderGeneral, Date, Time structs
      HeaderSignal.hpp                # HeaderSignal struct
      HeaderUtils.hpp                 # CreateHeaderGeneral, CreateHeaderSignal
    reader/
      ReaderHeaderExam.hpp            # ReadHeaderExam() entry point
    writer/
      WriterHeaderExam.hpp            # WriteHeaderExam() entry point
    store/                            # Read iterators
      DataRecordStore.hpp             # Iterate full data records
      SignalRecordStore.hpp           # Iterate one signal's records
      SignalSampleStore.hpp           # Iterate individual samples
      TimeStampStore.hpp              # Iterate EDF+ timestamps
      TalStore.hpp                    # Iterate TALs within a record
      StoreUtils.hpp                  # Factory functions for stores
    sink/                             # Write iterators
      DataRecordSink.hpp              # Write full data records
      SignalRecordSink.hpp            # Write one signal's records
      SinkUtils.hpp                   # Factory functions for sinks
    processor/                        # Data transformation
      ProcessorSampleRecord.hpp       # Raw bytes -> physical/digital values
      ProcessorSample.hpp             # Physical/digital values -> raw bytes
      ProcessorTalRecord.hpp          # TAL bytes -> vector<Annotation>
      ProcessorAnnotation.hpp         # Annotation -> TAL record
      ProcessorTimeStamp.hpp          # TimeStamp -> record
      ProcessorTimeStampRecord.hpp    # Record -> TimeStamp
  tests/                              # doctest-based test suite
  files/                              # Sample EDF/BDF files for testing
  third_party/                        # Bundled doctest headers
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file
for details.

Copyright (c) 2017-present Iuri Dotta

## Contributing

Contributions are welcome. To get started:

1. Fork the repository at [github.com/idotta/edfio](https://github.com/idotta/edfio)
2. Create a feature branch from `master`
3. Make sure all tests pass (`ctest --test-dir build`)
4. Keep changes focused -- one feature or fix per pull request
5. Follow existing code style and naming conventions
6. Open a pull request with a clear description of the change
