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

	struct Annotation
	{
		long long m_start = 0; // expressed in units of 100 nanoSeconds
		long long m_duration = 0; // expressed in units of 100 nanoSeconds
		std::string m_annotation;
	};

}
