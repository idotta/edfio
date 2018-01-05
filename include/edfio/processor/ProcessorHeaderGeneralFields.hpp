//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../header/HeaderGeneral.hpp"

namespace edfio
{

	struct ProcessorHeaderGeneralFields
	{
		using In = HeaderGeneralFields;
		using Out = HeaderGeneral;
		Out operator ()(In in);
	};

}

#include "impl/ProcessorHeaderGeneralFields.ipp"