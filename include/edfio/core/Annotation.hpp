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
	//    Each TAL starts with a time stamp Onset21Duration20

	static const char DURATION_DIV = 21;
	static const char ANNOTATION_DIV = 20;
	static const char ANNOTATION_END = 0;

	struct Annotation
	{
		long long m_onset; // expressed in units of 100 nanoSeconds
		long long m_duration; // expressed in units of 100 nanoSeconds
		std::string m_annotation;
	};

}
