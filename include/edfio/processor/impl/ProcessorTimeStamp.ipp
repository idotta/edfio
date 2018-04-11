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
#include "../../core/Annotation.hpp"
#include "../detail/ProcessorUtils.hpp"

#include <vector>

namespace edfio
{

	inline Record<char> ProcessorTimeStamp::operator()(TimeStamp timestamp)
	{
		std::string ts = detail::to_string_decimal(timestamp.m_start);

		if (ts.empty())
		{
			throw std::invalid_argument(detail::GetError(FileErrc::FileWriteInvalidAnnotations));
		}

		size_t plusSignal = 0;
		if (timestamp.m_start >= 0)
			plusSignal = 1;

		Record<char> record(plusSignal + ts.size() + 3); // 20 20 0
		auto it = record().begin();

		// timestamp
		if (plusSignal)
			*it++ = '+';
		std::move(ts.begin(), ts.end(), it);
		it += ts.size();
		*it++ = 20; // 20 div
		*it++ = 20; // 20 div
		*it++ = 0; // 0 end

		return record;
	}

}
