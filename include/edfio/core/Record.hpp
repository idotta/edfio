//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include <iostream>
#include <vector>

namespace edfio
{

	// This structure can be used for either single signal record or data record IO
	// It is important that a data record always has constant size
	struct RecordField
	{
		RecordField() = default;

		RecordField(size_t recordSize)
			: m_recordSize(recordSize)
			, m_record(recordSize) {}

		RecordField(std::vector<char>::const_iterator first, std::vector<char>::const_iterator last)
			: m_recordSize(std::distance(first, last))
			, m_record(first, last) {}

		const size_t Size() const
		{
			return m_recordSize;
		}
		const std::vector<char>& operator()() const
		{
			return m_record;
		}
		std::vector<char>& operator()()
		{
			return m_record;
		}
	private:
		std::vector<char> m_record;
		size_t m_recordSize;
	};

	std::ostream& operator << (std::ostream &os, RecordField &rf)
	{
		auto &record = rf();
		record.resize(rf.Size());
		os.write(record.data(), rf.Size());
		return os;
	}

	std::istream& operator >> (std::istream &is, RecordField &rf)
	{
		auto &record = rf();
		record.resize(rf.Size());
		is.read(&record[0], rf.Size());
		return is;
	}

}
