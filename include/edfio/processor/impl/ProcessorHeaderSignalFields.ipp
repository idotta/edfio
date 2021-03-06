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

namespace edfio
{

	inline std::vector<HeaderSignal> ProcessorHeaderSignalFields::operator()(std::vector<HeaderSignalFields> in)
	{
		std::vector<HeaderSignal> signals(in.size());

		for (auto& sigFields : in)
		{
			if (detail::CheckFormatErrors(sigFields.m_label())
				|| detail::CheckFormatErrors(sigFields.m_transducer())
				|| detail::CheckFormatErrors(sigFields.m_physDimension())
				|| detail::CheckFormatErrors(sigFields.m_physicalMin())
				|| detail::CheckFormatErrors(sigFields.m_physicalMax())
				|| detail::CheckFormatErrors(sigFields.m_digitalMin())
				|| detail::CheckFormatErrors(sigFields.m_digitalMax())
				|| detail::CheckFormatErrors(sigFields.m_prefilter())
				|| detail::CheckFormatErrors(sigFields.m_samplesInDataRecord())
				|| detail::CheckFormatErrors(sigFields.m_reserved()))
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}

		}

		// Labels
		{
			size_t totalAnnotationChannels = 0;
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& label = in[idx].m_label();
				if (IsPlus(m_version))
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
			}
			if (IsPlus(m_version) && totalAnnotationChannels == 0)
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
			// TODO: check if this is true
			/*if (m_datarecordDuration < 1)
			{
				if (signals.size() != totalAnnotationChannels || !IsPlus(m_version))
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
			}*/
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
						throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
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
				catch (...)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
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
				catch (...)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
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
				catch (...)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}

				if (signal.m_detail.m_isAnnotation)
				{
					if (IsEdf(m_version) && IsPlus(m_version))
					{
						if (n != -32768)
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
						}
					}
					else if (IsBdf(m_version) && IsPlus(m_version))
					{
						if (n != -8388608)
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
						}
					}
				}
				else if (IsEdf(m_version))
				{
					if ((n > 32767) || (n < -32768))
					{
						throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
					}
				}
				else if (IsBdf(m_version))
				{
					if ((n > 8388607) || (n < -8388608))
					{
						throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
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
				catch (...)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}

				if (signal.m_detail.m_isAnnotation)
				{
					if (IsEdf(m_version) && IsPlus(m_version))
					{
						if (n != 32767)
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
						}
					}
					else if (IsBdf(m_version) && IsPlus(m_version))
					{
						if (n != 8388607)
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
						}
					}
				}
				else if (IsEdf(m_version))
				{
					if ((n > 32767) || (n < -32768))
					{
						throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
					}
				}
				else if (IsBdf(m_version))
				{
					if ((n > 8388607) || (n < -8388608))
					{
						throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
					}
				}
				signal.m_digitalMax = n;
				if (signal.m_digitalMax < (signal.m_digitalMin + 1))
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
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
						throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
					}
				}
			}
		}
		// Samples in each datarecord
		{
			for (size_t idx = 0; idx < signals.size(); idx++)
			{
				auto &signal = signals[idx];
				auto& nrSamples = in[idx].m_samplesInDataRecord();

				int n = 0;
				try
				{
					n = std::stoi(nrSamples);
				}
				catch (...)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}

				if (n < 1)
				{
					throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
				}
				signal.m_samplesInDataRecord = n;
			}
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

				signal.m_detail.m_signalOffset = n;
				if (IsBdf(m_version))
					n += signal.m_samplesInDataRecord * 3;
				else  if (IsEdf(m_version))
					n += signal.m_samplesInDataRecord * 2;

				signal.m_detail.m_scaling = (signal.m_physicalMax - signal.m_physicalMin) / (signal.m_digitalMax - signal.m_digitalMin);
				signal.m_detail.m_offset = signal.m_physicalMin - signal.m_detail.m_scaling * signal.m_digitalMin;
			}
		}


		for (auto &signal : signals)
		{
			signal.m_label = detail::ReduceString(signal.m_label);
			signal.m_transducer = detail::ReduceString(signal.m_transducer);
			signal.m_physDimension = detail::ReduceString(signal.m_physDimension);
			signal.m_prefilter = detail::ReduceString(signal.m_prefilter);
			signal.m_reserved = detail::ReduceString(signal.m_reserved);
		}

		return std::move(signals);
	}

}
