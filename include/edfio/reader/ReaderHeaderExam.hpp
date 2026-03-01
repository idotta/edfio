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
#include "../header/HeaderExam.hpp"
#include "../processor/ProcessorHeaderExam.hpp"
#include "../processor/ProcessorHeaderGeneralFields.hpp"
#include "../processor/ProcessorHeaderSignalFields.hpp"
#include "ReaderHeaderGeneral.hpp"
#include "ReaderHeaderSignal.hpp"

#include <stdexcept>

namespace edfio {

inline HeaderExam ReadHeaderExam(Reader<char>::Stream &stream) {
  // Read general fields
  auto generalFields = ReadHeaderGeneral(stream);
  // Process general fields
  auto general = ProcessHeaderGeneralFields(std::move(generalFields));

  // Read signal fields
  auto signalFields = ReadHeaderSignal(stream, general.m_totalSignals);
  // Process signal fields
  auto signals = ProcessHeaderSignalFields(
      std::move(signalFields), general.m_version, general.m_datarecordDuration);

  // Process header exam
  auto header = ProcessHeaderExam(std::move(general), std::move(signals));

  // File size
  {
    // get current position
    auto position = stream.tellg();
    // get length of file
    stream.seekg(0, stream.end);
    long long length = stream.tellg();
    // send back to previous position
    stream.seekg(position, stream.beg);

    if (length != (header.m_general.m_detail.m_recordSize *
                       header.m_general.m_datarecordsFile +
                   header.m_general.m_headerSize)) {
      throw std::invalid_argument(GetError(FileErrc::FileContainsFormatErrors));
    }
  }

  return header;
}

} // namespace edfio
