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

  // Labels
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_label(inSig.m_label);
    }
  }
  // Transducers Types
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_transducer(inSig.m_transducer);
    }
  }
  // Physical Dimensions
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_physDimension(inSig.m_physDimension);
    }
  }
  // Physical Minima
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_physicalMin(
          detail::to_string_decimal(inSig.m_physicalMin));
    }
  }
  // Physical Maxima
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_physicalMax(
          detail::to_string_decimal(inSig.m_physicalMax));
    }
  }
  // Digital Minima
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_digitalMin(std::to_string(inSig.m_digitalMin));
    }
  }
  // Digital Maxima
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_digitalMax(std::to_string(inSig.m_digitalMax));
    }
  }
  // Prefilter
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_prefilter(inSig.m_prefilter);
    }
  }
  // Samples in each datarecord
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_samplesInDataRecord(
          std::to_string(inSig.m_samplesInDataRecord));
    }
  }
  // Reserved
  {
    for (auto &&[outSig, inSig] : std::views::zip(out, signals)) {
      outSig.m_reserved(inSig.m_reserved);
    }
  }

  return out;
}

} // namespace edfio
