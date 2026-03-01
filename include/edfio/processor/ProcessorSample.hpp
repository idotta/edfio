//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/SampleType.hpp"
#include "../core/Record.hpp"

namespace edfio
{

	template <SampleType SampleT>
	struct ProcessorSample
	{
		using ProcType = typename impl::Sample<SampleT>::type;
		using DigiType = impl::Sample<SampleType::Digital>::type;
		using PhysType = impl::Sample<SampleType::Physical>::type;

		ProcessorSample(double offset, double scaling, size_t sampleSize)
			: m_offset(offset)
			, m_scaling(scaling)
			, m_sampleSize(sampleSize)
		{
		}

		Record<char> operator ()(ProcType sample);

	private:
		const double m_offset;
		const double m_scaling;
		size_t m_sampleSize;
	};

	template<SampleType SampleT>
	inline Record<char> ProcessorSample<SampleT>::operator()(ProcType sample)
	{
		DigiType value = 0;
		if (std::is_same<DigiType, ProcType>::value)
			value = static_cast<DigiType>(sample);
		else
			value = impl::ConvertSample(m_offset, m_scaling, sample);

		Record<char> record(m_sampleSize);
		auto it = record().begin();

		for (int count = m_sampleSize; count > 0; count--)
		{
			unsigned char tmp = (value >> (count - 1) * 8);
			*it++ = tmp;
		}

		return record;
	}

}
