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
#include "../core/DataFormat.hpp"
#include "../header/HeaderExam.hpp"

#include <cstdint>

namespace edfio {

inline HeaderExam ProcessHeaderExam(HeaderGeneral header,
                                    std::vector<HeaderSignal> signals) {
  // Record size
  uint32_t recordsize = 0;
  for (auto &signal : signals) {
    recordsize += signal.m_samplesInDataRecord;
  }

  if (IsBdf(header.m_version)) {
    recordsize *= 3;
    if (recordsize > 0xF00000) {
      throw std::invalid_argument(GetError(FileErrc::FileContainsFormatErrors));
    }
  } else {
    recordsize *= 2;
    if (recordsize > 0xA00000) {
      throw std::invalid_argument(GetError(FileErrc::FileContainsFormatErrors));
    }
  }
  header.m_detail.m_recordSize = recordsize;

  return HeaderExam{std::move(header), std::move(signals)};
}

} // namespace edfio
