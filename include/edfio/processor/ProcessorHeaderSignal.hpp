//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../header/HeaderSignal.hpp"

namespace edfio
{

	struct ProcessorHeaderSignal
	{
		using In = std::vector<HeaderSignal>;
		using Out = std::vector<HeaderSignalFields>;
		Out operator ()(In in);
	};

}

#include "impl/ProcessorHeaderSignal.ipp"
