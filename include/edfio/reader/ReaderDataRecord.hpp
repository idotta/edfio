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

	struct ReaderDataRecord : ReaderBase
	{
		using ReaderBase::ReaderBase;

		template <class Processor, class Container>
		FileErrc operator ()(HeaderExam &header, HeaderSignal &signal, Processor processor, Container container)
		{
			return FileErrc();
		}
	};

}
