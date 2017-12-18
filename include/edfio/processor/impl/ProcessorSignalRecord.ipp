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

namespace edfio
{

	template<SampleType SampleT>
	typename ProcessorSignalRecord<SampleT>::TypeO ProcessorSignalRecord<SampleT>::operator()(TypeI in)
	{
		auto signalRecord = std::move(in());
		
		TypeO signalData;
		signalData.reserve(signalRecord.size() / m_sampleSize);

		for (auto it = signalRecord.begin(); it != signalRecord.end();)
		{
			int digVal = 0;
			int count = 0;
			while (count < m_sampleSize)
			{
				if (count == 0 && *it < 0)
					digVal = -1;
				unsigned char tmp = *it++;
				digVal <<= 8;
				digVal |= tmp;
				count++;
			}

			if (SampleT == SampleType::Physical)
			{
				double physVal = m_signal.m_detail.m_scaling * static_cast<double>(digVal) + m_signal.m_detail.m_offset;
				signalData.emplace_back(physVal);
			}
			else
			{
				signalData.emplace_back(digVal);
			}
		}

		return signalData;
	}

}

