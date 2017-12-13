//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Defs.hpp"

#include <string>
#include <vector>
#include <cctype>
#include <regex>

namespace edfio
{

	static const char ADDITIONAL_SEPARATOR = '|';

	static bool CheckFormatErrors(const std::string &str)
	{
		for (auto& c : str)
		{
			if (!std::isprint(c))
			{
				return true;
			}
		}
		return false;
	}

	static bool CheckFormatErrors(const std::vector<char> &str)
	{
		for (auto& c : str)
		{
			if (!std::isprint(c))
			{
				return true;
			}
		}
		return false;
	}

	static int GetMonthFromString(const std::string &str)
	{
		static const std::vector<std::string> months = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
		for (size_t idx = 0; idx < months.size(); idx++)
		{
			if (str == months[idx])
			{
				return idx + 1;
			}
		}
		return 0;
	}

	static std::string ReduceString(const std::string &value)
	{
		return std::regex_replace(value, std::regex("^ +| +$|( ) +"), "$1");
	}

	template <class In, class Ou>
	struct ProcessorBase
	{
		using TypeIn = In;
		using TypeOu = Ou;

		virtual TypeOu operator ()(TypeIn in) = 0;
	};

}
