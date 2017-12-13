//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "ProcessorBase.hpp"
#include "../Defs.hpp"
#include "../header/HeaderExam.hpp"

namespace edfio
{

	struct ProcessorHeaderSignal : ProcessorBase<std::vector<HeaderSignalFields>, HeaderExam>
	{
		FileErrc operator ()(const TypeIn &in, TypeOu &ou);
	};

}

#include "impl/ProcessorHeaderSignal.ipp"
