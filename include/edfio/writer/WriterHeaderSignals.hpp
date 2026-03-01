//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Errors.hpp"
#include "../core/StreamIO.hpp"
#include "../header/HeaderSignal.hpp"

#include <span>
#include <stdexcept>

namespace edfio {

inline void WriteHeaderSignals(Writer<char>::Stream &stream,
                               std::span<HeaderSignalFields> signals) {
  if (!stream || !stream.is_open())
    throw std::invalid_argument(GetError(FileErrc::FileNotOpened));
  try {
    stream.clear();
    stream.seekp(256, std::ios::beg);

    for (auto &s : signals)
      stream << s.m_label;
    for (auto &s : signals)
      stream << s.m_transducer;
    for (auto &s : signals)
      stream << s.m_physDimension;
    for (auto &s : signals)
      stream << s.m_physicalMin;
    for (auto &s : signals)
      stream << s.m_physicalMax;
    for (auto &s : signals)
      stream << s.m_digitalMin;
    for (auto &s : signals)
      stream << s.m_digitalMax;
    for (auto &s : signals)
      stream << s.m_prefilter;
    for (auto &s : signals)
      stream << s.m_samplesInDataRecord;
    for (auto &s : signals)
      stream << s.m_reserved;
  } catch (const std::exception &) {
    throw std::invalid_argument(GetError(FileErrc::FileWriteError));
  }
}

} // namespace edfio
