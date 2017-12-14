//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include <fstream>

namespace edfio
{

	template <class Stream, class Result>
	struct Streamer
	{
		using StreamT = Stream;
		using ResultT = Result;

		virtual ResultT operator ()(StreamT &stream) = 0;
	};

	template <class Result>
	using Reader = Streamer<std::ifstream, Result>;

	template <class Result>
	using Writer = Streamer<std::ofstream, Result>;

}
