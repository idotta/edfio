//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include <string>

namespace edfio
{

	namespace detail
	{
		//    Each TAL starts with a time stamp Onset21Duration20
		static const char DURATION_DIV = 21;
		static const char ANNOTATION_DIV = 20;
		static const char ANNOTATION_END = 0;
	}

	struct TimeStamp
	{
		long long m_dararecord = 0;
		double m_start = 0;
	};

	struct Annotation : TimeStamp
	{
		double m_duration = 0;
		std::string m_annotation;
	};

}
