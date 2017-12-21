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

	template<>
	ProcessorSignalRecord<SampleType::Digital>::ThisType ProcessorSignalRecord<SampleType::Digital>::CalcSample(DigiType sample)
	{
		return sample;
	}

	template<>
	ProcessorSignalRecord<SampleType::Digital>::ThisType ProcessorSignalRecord<SampleType::Digital>::CalcSample(PhysType sample)
	{
		return static_cast<ThisType>((sample - m_signal.m_detail.m_offset) / m_signal.m_detail.m_scaling);
	}

	template<>
	ProcessorSignalRecord<SampleType::Physical>::ThisType ProcessorSignalRecord<SampleType::Physical>::CalcSample(DigiType sample)
	{
		return m_signal.m_detail.m_scaling * static_cast<double>(sample) + m_signal.m_detail.m_offset;
	}

	template<>
	ProcessorSignalRecord<SampleType::Physical>::ThisType ProcessorSignalRecord<SampleType::Physical>::CalcSample(PhysType sample)
	{
		return (sample - m_signal.m_detail.m_offset) / m_signal.m_detail.m_scaling;
	}

	template<SampleType SampleT>
	typename ProcessorSignalRecord<SampleT>::TypeO ProcessorSignalRecord<SampleT>::operator << (TypeI in)
	{
		auto signalRecord = std::move(in());

		TypeO signalData;
		signalData.reserve(signalRecord.size() / m_sampleSize);

		for (auto it = signalRecord.begin(); it != signalRecord.end();)
		{
			int sample = 0;
			int count = 0;
			while (count < m_sampleSize)
			{
				if (count == 0 && *it < 0)
					sample = -1;
				unsigned char tmp = *it++;
				sample <<= 8;
				sample |= tmp;
				count++;
			}
			signalData.emplace_back(CalcSample(sample));
		}

		return signalData;
	}

	template<SampleType SampleT>
	typename ProcessorSignalRecord<SampleT>::TypeI ProcessorSignalRecord<SampleT>::operator >> (TypeO in)
	{
		auto signalData = std::move(in);
		TypeI signalRecord(signalData.size() * m_sampleSize);
		auto &record = signalRecord();
		//record.reserve(signalData.size() * m_sampleSize);
		auto it = record.begin();

		for (auto &v : signalData)
		{
			int sample = static_cast<int>(CalcSample(v));
			for (int count = m_sampleSize; count > 0; count--)
			{
				unsigned char tmp = (sample >> (count - 1) * 8);
				//record.emplace_back(tmp);
				*it++ = tmp;
			}
		}

		return std::move(signalRecord);
	}

}

