//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "ReaderBase.hpp"
#include "../Defs.hpp"
#include "../header/HeaderExam.hpp"
#include "../header/HeaderSignal.hpp"

#include <vector>

namespace edfio
{

	struct ReaderHeaderExam : ReaderBase
	{
		using ReaderBase::ReaderBase;
		FileErrc operator ()(HeaderExamFields &hdr);
	};

	struct ReaderHeaderSignal : ReaderBase
	{
		using ReaderBase::ReaderBase;
		FileErrc operator ()(std::vector<HeaderSignalFields> &signals);
	};

}

#include "impl/ReaderHeader.ipp"