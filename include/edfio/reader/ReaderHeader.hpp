//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Defs.hpp"
#include "../core/Streamer.hpp"
#include "../header/HeaderGeneral.hpp"
#include "../header/HeaderSignal.hpp"

#include <vector>

namespace edfio
{

	struct ReaderHeaderGeneral : Streamer<std::ifstream>
	{
		using Streamer::Streamer;
		HeaderGeneralFields operator ()();
	};

	struct ReaderHeaderSignal : Streamer<std::ifstream>
	{
		using Streamer::Streamer;
		std::vector<HeaderSignalFields> operator ()(size_t totalSignals);
	};

}

#include "impl/ReaderHeader.ipp"
