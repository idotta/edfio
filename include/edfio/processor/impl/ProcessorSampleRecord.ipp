//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

namespace edfio
{

	template<SampleType SampleT>
	inline typename ProcessorSampleRecord<SampleT>::ProcType ProcessorSampleRecord<SampleT>::operator()(Record<char> record)
	{
		DigiType sample = 0;
		size_t idx = 0;
		for (unsigned char r : record())
		{
			if (r < 0 && idx++)
				sample = -1;
			sample <<= 8;
			sample |= r;
			idx++;
		}

		if (std::is_same<DigiType, ProcType>::value)
			return sample;
		return impl::ConvertSample(m_offset, m_scaling, sample);
	}

}

