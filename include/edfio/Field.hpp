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

	template <class Type, size_t Sz>
	struct Field
	{
		std::string m_valueStr;
		Type m_value;
		constexpr size_t Size() { return Sz; }
	};

	template <class Type, size_t Sz>
	std::ostream& operator << (std::ostream &os, Field<Type, Sz> &f)
	{
		auto &value = f.m_valueStr;

		if (Sz < value.size())
		{
			os.write(value.data(), Sz);
		}
		else
		{
			auto value = f.m_value;
			value.resize(Sz, ' ');
			os.write(value.data(), Sz);
		}

		return os;
	}

	template <class Type, size_t Sz>
	std::istream& operator >> (std::istream &is, Field<Type, Sz> &f)
	{
		auto &value = f.m_valueStr;
		value.resize(Sz);
		is.read(value.data(), Sz);

		return is;
	}

}
