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

	// Records have fixed sizes that can vary according to the signal
	// They can be used for either single signal record or data record IO
	// It is important that data records always have one size in the same file
	template <typename ValT = char>
	struct Record
	{
		using ValueType = ValT;
		using VectorType = std::vector<ValueType>;

		Record() = delete;

		Record(size_t recordSize)
			: m_size(recordSize)
			, m_value(recordSize, 0) {}

		Record(typename VectorType::const_iterator first, typename VectorType::const_iterator last)
			: m_size(std::distance(first, last))
			, m_value(first, last) {}

		Record(const Record<ValT> &record)
			: m_size(record.m_size)
			, m_value(record.m_value)
		{
		}

		const size_t Size() const
		{
			return m_size;
		}
		const VectorType& operator()() const
		{
			return m_value;
		}
		VectorType& operator()()
		{
			return m_value;
		}
		Record<ValueType> operator+(const Record<ValueType>& record)
		{
			Record<ValueType> tmp(Size() + record.Size());
			std::copy(m_value.begin(), m_value.end(), tmp().begin());
			std::copy(record().begin(), record().end(), tmp().begin() + Size());
			return tmp;
		}

		VectorType m_value;
		const size_t m_size;
	};

	template <typename ValT = char>
	std::ostream& operator << (std::ostream &os, Record<ValT> &r)
	{
		auto &record = r();
		record.resize(r.Size(), 0);
		os.write(record.data(), r.Size() * sizeof(ValT));
		return os;
	}

	template <typename ValT = char>
	std::istream& operator >> (std::istream &is, Record<ValT> &r)
	{
		auto &record = r();
		record.resize(r.Size(), 0);
		is.read(&record[0], r.Size()  * sizeof(ValT));
		return is;
	}

}
