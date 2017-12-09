//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "Defs.hpp"
#include "Reader.hpp"
#include "ExamHeader.hpp"
#include "SignalHeader.hpp"

#include <vector>

namespace edfio
{

	struct ExamHeaderReader : Reader
	{
		using Reader::Reader;
		FileErrc operator ()(ExamHeader &hdr);
	};

	struct SignalHeaderReader : Reader
	{
		using Reader::Reader;
		FileErrc operator ()(std::vector<SignalHeader> &signals);
	};

}

#include "impl/HeaderReader.ipp"