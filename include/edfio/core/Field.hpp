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

	template <size_t Sz>
	struct Field
	{
		std::string m_value;
		constexpr size_t Size() const
		{ 
			return Sz; 
		}
		const std::string& operator()() const
		{
			return m_value;
		}
		std::string& operator()()
		{
			return m_value;
		}
	};

	template <size_t Sz>
	std::ostream& operator << (std::ostream &os, Field<Sz> &f)
	{
		auto &value = f();
		value.resize(Sz);
		os.write(value.data(), Sz);
		return os;
	}

	template <size_t Sz>
	std::istream& operator >> (std::istream &is, Field<Sz> &f)
	{
		auto &value = f();
		value.resize(Sz);
		is.read(&value[0], Sz);
		return is;
	}

}
