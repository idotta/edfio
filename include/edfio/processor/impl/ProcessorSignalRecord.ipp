//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../header/HeaderSignal.hpp"

#include <bitset>

namespace edfio
{

	namespace impl
	{
		double ConvertToPhysicalValue(const HeaderSignal& signal, long value)
		{
			return signal.m_detail.m_bitValue * (signal.m_detail.m_offset + static_cast<double>(value));
		}
	}

	template<SampleType SampleT>
	typename ProcessorSignalRecord<SampleT>::TypeOu ProcessorSignalRecord<SampleT>::operator()(TypeIn in)
	{
		auto signalRecord = std::move(in());
		
		TypeOu signalData;
		signalData.reserve(signalRecord.size() / m_sampleSize);

		for (auto it = signalRecord.begin(); it != signalRecord.end();)
		{
			std::bitset<32> value;
			int count = 0;
			while (count < m_sampleSize)
			{
				std::bitset<32> tmp(*it++);
				value <<= 8;
				value |= tmp;
				count++;
			}

			if (SampleT == SampleType::Physical)
			{
				signalData.emplace_back(impl::ConvertToPhysicalValue(m_signal, value.to_ullong()));
			}
			else
			{
				signalData.emplace_back(static_cast<int>(value.to_ullong()));
			}
		}

		return signalData;
	}

}

