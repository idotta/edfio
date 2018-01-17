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
#include "../header/HeaderSignal.hpp"
#include "../core/StreamIO.hpp"

#include <vector>

namespace edfio
{

	struct ReaderHeaderSignal : Reader<char>
	{
		ReaderHeaderSignal(size_t totalSignals) : m_totalSignals(totalSignals) {}

		std::vector<HeaderSignalFields> operator ()(Stream &stream);
	private:
		size_t m_totalSignals = 0;
	};

}

#include "impl/ReaderHeaderSignal.ipp"
