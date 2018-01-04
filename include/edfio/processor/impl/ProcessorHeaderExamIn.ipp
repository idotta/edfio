//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Utils.hpp"
#include "../../core/DataFormat.hpp"

namespace edfio
{

	ProcessorHeaderExamIn::Out edfio::ProcessorHeaderExamIn::operator()(In in)
	{
		auto &header = in.first;
		auto &signals = in.second;
		
		// Record size
		size_t recordsize = 0;
		for (auto &signal : signals)
		{
			recordsize += signal.m_samplesInDataRecord;
		}

		if (IsBdf(header.m_version))
		{
			recordsize *= 3;
			if (recordsize > 0xF00000)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		else
		{
			recordsize *= 2;
			if (recordsize > 0xA00000)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}
		header.m_detail.m_recordSize = recordsize;

		return std::move(Out{ std::move(header), std::move(signals) });
	}

}
