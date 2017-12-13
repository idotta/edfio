//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Defs.hpp"
#include "../../header/HeaderExam.hpp"
#include "../../header/HeaderSignal.hpp"

#include <vector>
#include <sstream>
#include <cctype>
#include <algorithm>

namespace edfio
{

	FileErrc ProcessorHeaderSignal::operator()(const std::vector<HeaderSignalFields> & in, std::pair<HeaderExam, std::vector<HeaderSignal>>& ou)
	{
		auto& header = ou.first;
		auto& signals = ou.second;

		signals.resize(in.size());

		for (auto& sigFields : in)
		{
			if (CheckFormatErrors(sigFields.m_label())
				|| CheckFormatErrors(sigFields.m_transducer())
				|| CheckFormatErrors(sigFields.m_physDimension())
				|| CheckFormatErrors(sigFields.m_physicalMin())
				|| CheckFormatErrors(sigFields.m_physicalMax())
				|| CheckFormatErrors(sigFields.m_digitalMin())
				|| CheckFormatErrors(sigFields.m_digitalMax())
				|| CheckFormatErrors(sigFields.m_prefilter())
				|| CheckFormatErrors(sigFields.m_samplesInDataRecord())
				|| CheckFormatErrors(sigFields.m_reserved()))
			{
				return FileErrc::FileContainsFormatErrors;
			}

		}

		// Labels
		{
			size_t totalAnnotationChannels = 0;
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& label = in[idx].m_label();
				if (IsPlus(header.m_version))
				{
					if (label.find("Annotation") != std::string::npos)
					{
						totalAnnotationChannels++;
						signal.m_detail.m_isAnnotation = true;
					}
					else
					{
						signal.m_detail.m_isAnnotation = false;
					}
				}
				else
				{
					signal.m_detail.m_isAnnotation = false;
				}
				signal.m_label = label;
				signal.m_detail.m_signalIndex = idx;
			}
			if (IsPlus(header.m_version) && totalAnnotationChannels == 0)
			{
				return FileErrc::FileContainsFormatErrors;
			}
			if (signals.size() != totalAnnotationChannels || !IsPlus(header.m_version))
			{
				if (header.m_datarecordDuration < 1)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Transducers Types
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& transducer = in[idx].m_transducer();

				signal.m_transducer = transducer;

				if (signal.m_detail.m_isAnnotation)
				{
					if (transducer.find_first_not_of(' ') != std::string::npos)
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
			}
		}
		// Physical Dimensions
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& physDimension = in[idx].m_physDimension();

				signal.m_physDimension = physDimension;
			}
		}
		// Physical Minima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& physMin = in[idx].m_physicalMin();

				try
				{
					signal.m_physicalMin = std::stod(physMin);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Physical Maxima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& physMax = in[idx].m_physicalMax();

				try
				{
					signal.m_physicalMax = std::stod(physMax);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Digital Minima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& digMin = in[idx].m_digitalMin();

				int n = 0;
				try
				{
					n = std::stoi(digMin);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				if (signal.m_detail.m_isAnnotation)
				{
					if (IsEdf(header.m_version) && IsPlus(header.m_version))
					{
						if (n != -32768)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
					else if (IsBdf(header.m_version) && IsPlus(header.m_version))
					{
						if (n != -8388608)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
				}
				else if (IsEdf(header.m_version))
				{
					if ((n > 32767) || (n < -32768))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				else if (IsBdf(header.m_version))
				{
					if ((n > 8388607) || (n < -8388608))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				signal.m_digitalMin = n;
			}
		}
		// Digital Maxima
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& digMax = in[idx].m_digitalMax();

				int n = 0;
				try
				{
					n = std::stoi(digMax);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				if (signal.m_detail.m_isAnnotation)
				{
					if (IsEdf(header.m_version) && IsPlus(header.m_version))
					{
						if (n != 32767)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
					else if (IsBdf(header.m_version) && IsPlus(header.m_version))
					{
						if (n != 8388607)
						{
							return FileErrc::FileContainsFormatErrors;
						}
					}
				}
				else if (IsEdf(header.m_version))
				{
					if ((n > 32767) || (n < -32768))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				else if (IsBdf(header.m_version))
				{
					if ((n > 8388607) || (n < -8388608))
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
				signal.m_digitalMax = n;
				if (signal.m_digitalMax < (signal.m_digitalMin + 1))
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
		}
		// Prefilter
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& prefilter = in[idx].m_prefilter();

				signal.m_prefilter = prefilter;

				if (signal.m_detail.m_isAnnotation)
				{
					if (prefilter.find_first_not_of(' ') != std::string::npos)
					{
						return FileErrc::FileContainsFormatErrors;
					}
				}
			}
		}
		// Samples in each datarecord
		{
			size_t recordsize = 0;

			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& nrSamples = in[idx].m_samplesInDataRecord();

				int n = 0;
				try
				{
					n = std::stoi(nrSamples);
				}
				catch (std::exception& e)
				{
					return FileErrc::FileContainsFormatErrors;
				}

				if (n < 1)
				{
					return FileErrc::FileContainsFormatErrors;
				}
				signal.m_samplesInDataRecord = n;
				signal.m_detail.m_samplesInFile = n * header.m_datarecordsFile;
				recordsize += n;
			}

			if (IsBdf(header.m_version))
			{
				recordsize *= 3;
				if (recordsize > 0xF00000)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
			else
			{
				recordsize *= 2;
				if (recordsize > 0xA00000)
				{
					return FileErrc::FileContainsFormatErrors;
				}
			}
			header.m_detail.m_recordSize = recordsize;
		}
		// Reserved
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& reserved = in[idx].m_reserved();
				signal.m_reserved = reserved;
			}
		}
		// Details
		{
			size_t n = 0;
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];

				signal.m_detail.m_bufferOffset = n;
				if (IsBdf(header.m_version))
					n += signal.m_samplesInDataRecord * 3;
				else  if (IsEdf(header.m_version))
					n += signal.m_samplesInDataRecord * 2;

				signal.m_detail.m_bitValue = (signal.m_physicalMax - signal.m_physicalMin) / (signal.m_digitalMax - signal.m_digitalMin);
				signal.m_detail.m_offset = signal.m_physicalMax / signal.m_detail.m_bitValue - signal.m_digitalMax;
			}
		}


		for (auto &signal : signals)
		{
			signal.m_label = ReduceString(signal.m_label);
			signal.m_transducer = ReduceString(signal.m_transducer);
			signal.m_physDimension = ReduceString(signal.m_physDimension);
			signal.m_prefilter = ReduceString(signal.m_prefilter);
			signal.m_reserved = ReduceString(signal.m_reserved);
		}

		return FileErrc::NoError;
	}

}
