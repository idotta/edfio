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
#include "../detail/ProcessorUtils.hpp"

#include <vector>
#include <string>

namespace edfio
{

	std::vector<HeaderSignalFields> ProcessorHeaderSignal::operator()(std::vector<HeaderSignal> in)
	{
		std::vector<HeaderSignalFields> out(in.size());
		auto &signals = in;

		// Labels
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_label(signals[idx].m_label);
			}
		}
		// Transducers Types
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_transducer(signals[idx].m_transducer);
			}
		}
		// Physical Dimensions
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_physDimension(signals[idx].m_physDimension);
			}
		}
		// Physical Minima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_physicalMin(detail::to_string_decimal(signals[idx].m_physicalMin));
			}
		}
		// Physical Maxima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_physicalMax(detail::to_string_decimal(signals[idx].m_physicalMax));
			}
		}
		// Digital Minima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_digitalMin(std::to_string(signals[idx].m_digitalMin));
			}
		}
		// Digital Maxima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_digitalMax(std::to_string(signals[idx].m_digitalMax));
			}
		}
		// Prefilter
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_prefilter(signals[idx].m_prefilter);
			}
		}
		// Samples in each datarecord
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_samplesInDataRecord(std::to_string(signals[idx].m_samplesInDataRecord));
			}
		}
		// Reserved
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				out[idx].m_reserved(signals[idx].m_reserved);
			}
		}

		return std::move(out);
	}

}
