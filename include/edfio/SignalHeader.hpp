//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "Header.hpp"
#include "Field.hpp"

#include <vector>

namespace edfio
{

	namespace detail
	{
		struct SignalHeaderDetail
		{
			long long m_samplesInFile = 0;
			long m_bufferOffset = 0;
			double m_bitValue = 0;
			double m_offset = 0;
			bool m_isAnnotation = false;
		};
	}

	struct SignalHeader
	{
		Field<std::string, 16> m_label;
		Field<std::string, 80> m_transducer;
		Field<std::string, 8> m_physDimension;
		Field<double, 8> m_physicalMin;
		Field<double, 8> m_physicalMax;
		Field<int, 8> m_digitalMin;
		Field<int, 8> m_digitalMax;
		Field<std::string, 80> m_prefilter;
		Field<int, 8> m_samplesInDataRecord;
		Field<std::string, 32> m_reserved;
		detail::SignalHeaderDetail m_detail;
	};

}
