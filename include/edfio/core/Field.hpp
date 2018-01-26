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
#include <string>

namespace edfio
{

	// Fields have fixed predetermined sizes in the exam file
	// They are used to read the headers
	template <size_t Sz, typename CharT = char>
	struct Field
	{
		using ValueType = CharT;

		constexpr size_t Size() const
		{
			return Sz;
		}
		const std::basic_string<ValueType>& operator()() const
		{
			return m_value;
		}
		std::basic_string<ValueType>& operator()()
		{
			return m_value;
		}
		void operator()(const std::basic_string<ValueType>& value)
		{
			m_value = value;
			m_value.resize(Size(), ' ');
		}

		std::basic_string<ValueType> m_value;
	};

	template <size_t Sz, typename CharT>
	std::ostream& operator << (std::ostream &os, Field<Sz, CharT> &f)
	{
		auto &value = f();
		value.resize(Sz, ' ');
		os.write(value.data(), Sz);
		return os;
	}

	template <size_t Sz, typename CharT>
	std::istream& operator >> (std::istream &is, Field<Sz, CharT> &f)
	{
		auto &value = f();
		value.resize(f.Size(), ' ');
		is.read(&value[0], f.Size());
		return is;
	}

}
