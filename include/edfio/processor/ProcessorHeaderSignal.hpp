//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../header/HeaderSignal.hpp"
#include "ProcessorUtils.hpp"

#include <ranges>
#include <string>
#include <vector>

namespace edfio {

inline std::vector<HeaderSignalFields>
ProcessHeaderSignal(std::vector<HeaderSignal> in) {
  std::vector<HeaderSignalFields> out(in.size());
  auto &signals = in;

  for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
    outSig.m_label(inSig.m_label);
    outSig.m_transducer(inSig.m_transducer);
    outSig.m_physDimension(inSig.m_physDimension);
    outSig.m_physicalMin(detail::to_string_decimal(inSig.m_physicalMin));
    outSig.m_physicalMax(detail::to_string_decimal(inSig.m_physicalMax));
    outSig.m_digitalMin(std::to_string(inSig.m_digitalMin));
    outSig.m_digitalMax(std::to_string(inSig.m_digitalMax));
    outSig.m_prefilter(inSig.m_prefilter);
    outSig.m_samplesInDataRecord(std::to_string(inSig.m_samplesInDataRecord));
    outSig.m_reserved(inSig.m_reserved);
  }

  return out;
}

} // namespace edfio
