//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../header/HeaderExam.hpp"
#include "../core/StreamIO.hpp"

#include <vector>

namespace edfio
{

	struct WriterHeaderExam : Writer<char>
	{
		void operator ()(Stream &stream, HeaderExam &input);
	};

}

#include "impl/WriterHeaderExam.ipp"
