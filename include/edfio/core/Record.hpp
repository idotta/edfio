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

	struct Record
	{
		HeaderSignal &m_signal;
		long long m_curSample = 0;

		Record(HeaderSignal &signal) : m_signal(signal) {}
	};

}
