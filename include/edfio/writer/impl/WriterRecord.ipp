//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Utils.hpp"
#include "../../core/Record.hpp"

#include <stdexcept>

namespace edfio
{

	void WriterRecord::operator()(Stream &stream, Record<char> &record)
	{
		if (!stream || !stream.is_open())
			throw std::invalid_argument(detail::GetError(FileErrc::FileNotOpened));

		try
		{
			stream << record;
		}
		catch (std::exception e)
		{
			throw std::invalid_argument(detail::GetError(FileErrc::FileWriteError));
		}
	}

}
