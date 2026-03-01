//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "EdfIO.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace edfio {

/// High-level facade for reading EDF/BDF files.
///
/// Owns the underlying file stream and parsed header.  Provides convenient
/// methods to extract physical or digital samples and annotations without
/// manually wiring stores, sinks, processors, and streams.
///
/// Moveable but not copyable (owns an std::ifstream).
class EdfFile {
public:
  EdfFile(const EdfFile &) = delete;
  EdfFile &operator=(const EdfFile &) = delete;
  EdfFile(EdfFile &&) noexcept = default;
  EdfFile &operator=(EdfFile &&) noexcept = default;

  /// Open an EDF/BDF file for reading.
  ///
  /// @param path  Filesystem path to the file.
  /// @throws std::runtime_error if the file cannot be opened.
  /// @throws std::invalid_argument if the file contains format errors.
  [[nodiscard]] static EdfFile open(const std::filesystem::path &path) {
    auto stream = std::make_unique<std::ifstream>(path, std::ios::binary);
    if (!stream->is_open()) {
      throw std::runtime_error(std::string("Cannot open file: ") +
                               path.string());
    }
    auto header = ReadHeaderExam(*stream);
    return EdfFile(std::move(stream), std::move(header));
  }

  // ---- Header access -------------------------------------------------------

  /// General header fields (patient, recording, timing, etc.).
  [[nodiscard]] const HeaderGeneral &general() const noexcept {
    return m_header.m_general;
  }

  /// Per-signal header fields.
  [[nodiscard]] const std::vector<HeaderSignal> &signals() const noexcept {
    return m_header.m_signals;
  }

  /// Full header exam (general + signals).
  [[nodiscard]] const HeaderExam &header() const noexcept { return m_header; }

  // ---- Format queries -------------------------------------------------------

  /// Data format enum value.
  [[nodiscard]] DataFormat format() const noexcept {
    return m_header.m_general.m_version;
  }

  /// True for any EDF variant (plain EDF, EDF+C, EDF+D).
  [[nodiscard]] bool isEdf() const noexcept { return IsEdf(format()); }

  /// True for any BDF variant (plain BDF, BDF+C, BDF+D).
  [[nodiscard]] bool isBdf() const noexcept { return IsBdf(format()); }

  /// True for any "plus" variant (EDF+C, EDF+D, BDF+C, BDF+D).
  [[nodiscard]] bool isPlus() const noexcept { return IsPlus(format()); }

  // ---- Counts and duration --------------------------------------------------

  /// Total number of signals (including annotation channels).
  [[nodiscard]] int32_t signalCount() const noexcept {
    return m_header.m_general.m_totalSignals;
  }

  /// Number of data records in the file.
  [[nodiscard]] int64_t dataRecordCount() const noexcept {
    return m_header.m_general.m_datarecordsFile;
  }

  /// Total file duration in seconds
  /// (dataRecordCount * datarecordDuration).
  [[nodiscard]] double duration() const noexcept {
    return m_header.m_general.m_detail.m_fileDuration;
  }

  // ---- Sample reading -------------------------------------------------------

  /// Read all physical-value samples for the given signal index.
  ///
  /// The returned vector contains one double per sample, converted from the
  /// raw digital value using the signal's offset and scaling parameters.
  ///
  /// @param signalIndex  Zero-based index into signals().
  /// @throws std::out_of_range if signalIndex is invalid.
  [[nodiscard]] std::vector<double> readSignal(size_t signalIndex) const {
    validateSignalIndex(signalIndex);

    auto const &sig = m_header.m_signals[signalIndex];
    auto store =
        detail::CreateSignalSampleStore(*m_stream, m_header.m_general, sig);

    ProcessorSampleRecord<SampleType::Physical> proc(sig.m_detail.m_offset,
                                                     sig.m_detail.m_scaling);

    std::vector<double> result;
    result.reserve(store.size());
    for (auto it = store.begin(); it != store.end(); ++it) {
      result.push_back(proc(*it));
    }
    return result;
  }

  /// Read all digital-value samples for the given signal index.
  ///
  /// The returned vector contains one int32_t per sample, which is the raw
  /// (sign-extended) integer stored in the file.
  ///
  /// @param signalIndex  Zero-based index into signals().
  /// @throws std::out_of_range if signalIndex is invalid.
  [[nodiscard]] std::vector<int32_t>
  readSignalDigital(size_t signalIndex) const {
    validateSignalIndex(signalIndex);

    auto const &sig = m_header.m_signals[signalIndex];
    auto store =
        detail::CreateSignalSampleStore(*m_stream, m_header.m_general, sig);

    // offset=0, scaling=1 yields the raw digital value.
    ProcessorSampleRecord<SampleType::Digital> proc(0.0, 1.0);

    std::vector<int32_t> result;
    result.reserve(store.size());
    for (auto it = store.begin(); it != store.end(); ++it) {
      result.push_back(proc(*it));
    }
    return result;
  }

  // ---- Annotations ----------------------------------------------------------

  /// Read all annotations from every annotation channel.
  ///
  /// Returns an empty vector when the file is not an EDF+/BDF+ variant or
  /// contains no annotation channels.
  [[nodiscard]] std::vector<Annotation> readAnnotations() const {
    std::vector<Annotation> result;

    for (size_t i = 0; i < m_header.m_signals.size(); ++i) {
      auto const &sig = m_header.m_signals[i];
      if (!sig.m_detail.m_isAnnotation)
        continue;

      auto sigStore =
          detail::CreateSignalRecordStore(*m_stream, m_header.m_general, sig);

      int64_t drIdx = 0;
      for (auto it = sigStore.begin(); it != sigStore.end(); ++it, ++drIdx) {
        auto const &rec = *it;
        std::vector<char> talData(rec().begin(), rec().end());
        if (talData.empty() ||
            (talData.front() != '+' && talData.front() != '-'))
          continue;

        auto annots = ProcessTalRecord(talData, drIdx);
        result.insert(result.end(), std::make_move_iterator(annots.begin()),
                      std::make_move_iterator(annots.end()));
      }
    }
    return result;
  }

  // ---- Low-level store access (for advanced use) ----------------------------

  /// Create a DataRecordStore over the file.
  [[nodiscard]] DataRecordStore dataRecordStore() const {
    return detail::CreateDataRecordStore(*m_stream, m_header.m_general);
  }

  /// Create a SignalRecordStore for the given signal index.
  ///
  /// @param signalIndex  Zero-based index into signals().
  /// @throws std::out_of_range if signalIndex is invalid.
  [[nodiscard]] SignalRecordStore signalRecordStore(size_t signalIndex) const {
    validateSignalIndex(signalIndex);
    return detail::CreateSignalRecordStore(*m_stream, m_header.m_general,
                                           m_header.m_signals[signalIndex]);
  }

  /// Create a SignalSampleStore for the given signal index.
  ///
  /// @param signalIndex  Zero-based index into signals().
  /// @throws std::out_of_range if signalIndex is invalid.
  [[nodiscard]] SignalSampleStore signalSampleStore(size_t signalIndex) const {
    validateSignalIndex(signalIndex);
    return detail::CreateSignalSampleStore(*m_stream, m_header.m_general,
                                           m_header.m_signals[signalIndex]);
  }

private:
  explicit EdfFile(std::unique_ptr<std::ifstream> stream, HeaderExam header)
      : m_stream(std::move(stream)), m_header(std::move(header)) {}

  void validateSignalIndex(size_t signalIndex) const {
    if (signalIndex >= m_header.m_signals.size()) {
      throw std::out_of_range("Signal index " + std::to_string(signalIndex) +
                              " out of range [0, " +
                              std::to_string(m_header.m_signals.size()) + ")");
    }
  }

  std::unique_ptr<std::ifstream> m_stream;
  HeaderExam m_header;
};

} // namespace edfio
