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


namespace edfio {

template <SampleType SampleT> struct ProcessorSampleRecord {
  using ProcType = typename Sample<SampleT>::type;
  using DigiType = Sample<SampleType::Digital>::type;
  using PhysType = Sample<SampleType::Physical>::type;

  ProcessorSampleRecord(double offset, double scaling)
      : m_offset(offset), m_scaling(scaling) {}

  ProcType operator()(Record<char> record);

private:
  const double m_offset;
  const double m_scaling;
};

template <SampleType SampleT>
inline typename ProcessorSampleRecord<SampleT>::ProcType
ProcessorSampleRecord<SampleT>::operator()(Record<char> record) {
  DigiType sample = 0;
  auto const &bytes = record();
  std::size_t const nbytes = bytes.size();

  // Assemble bytes (big-endian order as written by ProcessorSample)
  for (std::size_t i = 0; i < nbytes; ++i) {
    sample <<= 8;
    sample |= static_cast<unsigned char>(bytes[i]);
  }

  // Sign-extend: if high bit of the MSB is set, the value is negative
  if (nbytes > 0 && nbytes < sizeof(DigiType)) {
    unsigned int sign_bit = 1u << (nbytes * 8 - 1);
    if (sample & sign_bit)
      sample |= ~((1 << (nbytes * 8)) - 1);
  }

  if constexpr (std::is_same_v<DigiType, ProcType>)
    return sample;
  return ConvertSample(m_offset, m_scaling, sample);
}

} // namespace edfio
