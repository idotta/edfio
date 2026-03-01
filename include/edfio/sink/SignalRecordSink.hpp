//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "RecordSink.hpp"

namespace edfio {

class SignalRecordSink : public RecordSink {
public:
  using iterator = RecordSink::iterator;
  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  SignalRecordSink() = delete;

  SignalRecordSink(stream_type &stream, size_type recordSize,
                   size_type storeSize, std::streamoff headerOffset,
                   size_type datarecordSize, std::streamoff signalOffset)
      : RecordSink(stream, recordSize, storeSize, headerOffset),
        m_datarecordSize(datarecordSize), m_signalOffset(signalOffset) {
    measure();
  }

protected:
  void measure() override {
    auto pos = m_stream.tellp();
    m_stream.seekp(0, std::ios::end);
    size_type sz = m_stream.tellp();
    m_stream.seekp(pos);
    if (sz < static_cast<size_type>(m_headerOffset))
      throw std::invalid_argument("Invalid header");

    size_type hdrOff = static_cast<size_type>(m_headerOffset);
    size_type sigOff = static_cast<size_type>(m_signalOffset);
    size_type datarecords = (sz - hdrOff) / m_datarecordSize;
    size_type offset = (sz - hdrOff) % m_datarecordSize;

    m_sinkSize = datarecords;
    if (offset >= sigOff)
      m_sinkSize++;
  }

  void save(size_type off, value_type value) override {
    measure();
    if (off >= m_sinkSize) {
      m_stream.seekp(0, std::ios::end);
      size_type sz = m_stream.tellp();
      size_type hdrOff = static_cast<size_type>(m_headerOffset);
      size_type sigOff = static_cast<size_type>(m_signalOffset);
      size_type offset = (sz - hdrOff) % m_datarecordSize;
      if (offset < sigOff)
        throw std::invalid_argument("Invalid signal order");
      m_sinkSize++;
    } else {
      off = m_headerOffset + off * m_datarecordSize + m_signalOffset;
      m_stream.seekp(off, std::ios::beg);
    }
    m_value() = std::move(value());
    m_stream << m_value;
  }

  size_type m_datarecordSize;
  std::streamoff m_signalOffset;
};

} // namespace edfio
