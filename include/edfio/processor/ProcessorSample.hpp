//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/Record.hpp"
#include "../core/SampleType.hpp"

#include <cstdint>

namespace edfio {

template <SampleType SampleT> struct ProcessorSample {
  using ProcType = typename Sample<SampleT>::type;
  using DigiType = Sample<SampleType::Digital>::type;
  using PhysType = Sample<SampleType::Physical>::type;

  ProcessorSample(double offset, double scaling, uint32_t sampleSize)
      : m_offset(offset), m_scaling(scaling), m_sampleSize(sampleSize) {}

  Record<char> operator()(ProcType sample);

private:
  const double m_offset;
  const double m_scaling;
  uint32_t m_sampleSize;
};

template <SampleType SampleT>
inline Record<char> ProcessorSample<SampleT>::operator()(ProcType sample) {
  DigiType value = 0;
  if (std::is_same<DigiType, ProcType>::value)
    value = static_cast<DigiType>(sample);
  else
    value = ConvertSample(m_offset, m_scaling, sample);

  Record<char> record(m_sampleSize);
  auto it = record().begin();

  for (int32_t count = m_sampleSize; count > 0; count--) {
    uint8_t tmp = (value >> (count - 1) * 8);
    *it++ = tmp;
  }

  return record;
}

} // namespace edfio
