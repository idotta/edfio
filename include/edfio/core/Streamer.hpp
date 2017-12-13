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

	template <class Stream>
	struct Streamer
	{
		Stream &m_stream;
		Streamer(Stream &s) : m_stream(s) {}
	};

}
