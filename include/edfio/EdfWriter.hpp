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

namespace edfio {

/// High-level facade for writing EDF/BDF files.
///
/// Owns the underlying output stream.  Write data records sequentially after
/// the header has been written.  The data-record count in the header is
/// automatically patched on close() (or destruction).
///
/// Moveable but not copyable (owns an std::ofstream).
class EdfWriter {
public:
  EdfWriter(const EdfWriter &) = delete;
  EdfWriter &operator=(const EdfWriter &) = delete;
  EdfWriter(EdfWriter &&) noexcept = default;
  EdfWriter &operator=(EdfWriter &&) noexcept = default;

  /// Create a new EDF/BDF file and write its header.
  ///
  /// @param path    Filesystem path for the new file.
  /// @param header  Complete header exam to write.
  /// @throws std::runtime_error if the file cannot be created.
  [[nodiscard]] static EdfWriter create(const std::filesystem::path &path,
                                        const HeaderExam &header) {
    auto stream = std::make_unique<std::ofstream>(path, std::ios::binary);
    if (!stream->is_open()) {
      throw std::runtime_error(std::string("Cannot create file: ") +
                               path.string());
    }
    WriteHeaderExam(*stream, header);
    return EdfWriter(std::move(stream), header.m_general);
  }

  /// Write a single data record to the file.
  ///
  /// The data record count in the header is automatically updated when
  /// close() is called (or the writer is destroyed).
  void writeDataRecord(const Record<char> &record) {
    if (m_stream && m_stream->is_open()) {
      *m_stream << record;
      ++m_recordsWritten;
    }
  }

  /// Number of data records written so far.
  [[nodiscard]] int64_t recordsWritten() const noexcept {
    return m_recordsWritten;
  }

  /// Flush, patch the data-record count in the header, and close.
  void close() {
    if (m_stream && m_stream->is_open()) {
      // EDF/BDF header layout: the "number of data records" field is 8 bytes
      // starting at offset 236 (after version[8] + patient[80] +
      // recording[80] + startDate[8] + startTime[8] + headerSize[8] +
      // reserved[44] = 236).
      static constexpr std::streamoff kDataRecordCountOffset = 236;
      static constexpr size_t kFieldSize = 8;

      m_stream->seekp(kDataRecordCountOffset, std::ios::beg);
      auto countStr = std::to_string(m_recordsWritten);
      countStr.resize(kFieldSize, ' ');
      m_stream->write(countStr.data(), kFieldSize);

      m_stream->flush();
      m_stream->close();
    }
  }

  /// Destructor flushes and closes the stream if still open.
  ~EdfWriter() {
    try {
      close();
    } catch (...) {
      // Suppress exceptions in destructor.
    }
  }

private:
  explicit EdfWriter(std::unique_ptr<std::ofstream> stream,
                     HeaderGeneral general)
      : m_stream(std::move(stream)), m_general(std::move(general)) {}

  std::unique_ptr<std::ofstream> m_stream;
  HeaderGeneral m_general;
  int64_t m_recordsWritten = 0;
};

} // namespace edfio
