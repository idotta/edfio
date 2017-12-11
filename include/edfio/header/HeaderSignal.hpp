//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "HeaderBase.hpp"
#include "../Field.hpp"

namespace edfio
{

	struct HeaderSignalFields
	{
		Field<16> m_label;
		Field<80> m_transducer;
		Field<8> m_physDimension;
		Field<8> m_physicalMin;
		Field<8> m_physicalMax;
		Field<8> m_digitalMin;
		Field<8> m_digitalMax;
		Field<80> m_prefilter;
		Field<8> m_samplesInDataRecord;
		Field<32> m_reserved;
	};

	namespace detail
	{
		struct HeaderSignalDetail
		{
			long long m_samplesInFile = 0;
			long m_bufferOffset = 0;
			double m_bitValue = 0;
			double m_offset = 0;
			bool m_isAnnotation = false;
		};
	}

	struct HeaderSignal
	{
		// Field values
		std::string m_label;
		std::string m_transducer;
		std::string m_physDimension;
		double m_physicalMin = 0;
		double m_physicalMax = 0;
		int m_digitalMin = 0;
		int m_digitalMax = 0;
		std::string m_prefilter;
		int m_samplesInDataRecord = 0;
		std::string m_reserved;
		// Extra values
		detail::HeaderSignalDetail m_detail;
	};

}
