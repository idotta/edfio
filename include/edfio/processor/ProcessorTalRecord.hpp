//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/Annotation.hpp"

#include <vector>

namespace edfio
{

	struct ProcessorTalRecord
	{
		std::vector<Annotation> operator ()(std::vector<char> record, long long datarecord);
	};

}

#include "impl/ProcessorTalRecord.ipp"
