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
#include "../processor/ProcessorHeaderGeneral.hpp"
#include "../processor/ProcessorHeaderSignal.hpp"
#include "WriterHeaderGeneral.hpp"
#include "WriterHeaderSignals.hpp"

#include <vector>

namespace edfio {

inline void WriteHeaderExam(Writer<char>::Stream &stream, HeaderExam &input) {
  // Process header general
  auto general = ProcessHeaderGeneral(input.m_general);

  // Process signal fields
  auto signals = ProcessHeaderSignal(input.m_signals);

  // Write general
  WriteHeaderGeneral(stream, general);

  // Write signals
  WriteHeaderSignals(stream, signals);
}

} // namespace edfio
