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
#include "ExamHeader.hpp"
#include "SignalHeader.hpp"

#include <string>

namespace edfio
{

	template <typename Stream>
	class File
	{
	protected:
		Stream m_stream;
		ExamHeader m_header;
		std::vector<SignalHeader> m_signals;
		DataFormat m_format = DataFormat::Invalid;
	public:
		virtual FileErrc Open(const std::string &path) = 0;
		virtual FileErrc Close() = 0;
	};

	static const bool IsPlus(DataFormat format)
	{
		return format == DataFormat::EdfPlusC || format == DataFormat::EdfPlusD
			|| format == DataFormat::BdfPlusC || format == DataFormat::BdfPlusD;
	}

	static const bool IsEdf(DataFormat format)
	{
		return format == DataFormat::Edf || format == DataFormat::EdfPlusC || format == DataFormat::EdfPlusD;
	}

	static const bool IsBdf(DataFormat format)
	{
		return format == DataFormat::Bdf || format == DataFormat::BdfPlusC || format == DataFormat::BdfPlusD;
	}

}
