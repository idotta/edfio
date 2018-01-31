//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Config.hpp"

#include <string>
#include <vector>
#include <cctype>
#include <regex>

namespace edfio
{

	namespace impl
	{

		template <ProcessorErrorCheck Check, typename CharT>
		static bool CheckFormatErrors(const typename std::enable_if<Check == ProcessorErrorCheck::Strict, std::basic_string<CharT>>::type &str)
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

		template <ProcessorErrorCheck Check, typename CharT>
		static bool CheckFormatErrors(const typename std::enable_if<Check == ProcessorErrorCheck::Permissive, std::basic_string<CharT>>::type &str)
		{
			return false;
		}

		template <ProcessorErrorCheck Check, typename CharT>
		static bool CheckFormatErrors(const typename std::enable_if<Check == ProcessorErrorCheck::Strict, std::vector<CharT>>::type &str)
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

		template <ProcessorErrorCheck Check, typename CharT>
		static bool CheckFormatErrors(const typename std::enable_if<Check == ProcessorErrorCheck::Permissive, std::vector<CharT>>::type &str)
		{
			return false;
		}

	}

	namespace detail
	{

		static const char ADDITIONAL_SEPARATOR = '|';

		template <typename CharT>
		static bool CheckFormatErrors(const std::basic_string<CharT> &str)
		{
			return impl::CheckFormatErrors<config::PROCESSOR_ERROR_CHECKING, CharT>(str);
		}

		template <typename CharT>
		static bool CheckFormatErrors(const std::vector<CharT> &str)
		{
			return impl::CheckFormatErrors<config::PROCESSOR_ERROR_CHECKING, CharT>(str);
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

		static std::string GetStringFromMonth(size_t idx)
		{
			idx--;
			static const std::vector<std::string> months = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
			if (idx >= 0 && idx < months.size())
				return months[idx];
			return "JAN";
		}

		static std::string ReduceString(const std::string &value)
		{
			return std::regex_replace(value, std::regex("^ +| +$|( ) +"), "$1");
		}

		static std::string GetFormatName(DataFormat format)
		{
			if (format == DataFormat::Edf)
				return "EDF";
			if (format == DataFormat::EdfPlusC)
				return "EDF+C";
			if (format == DataFormat::EdfPlusD)
				return "EDF+D";
			if (format == DataFormat::Bdf)
				return "BDF";
			if (format == DataFormat::BdfPlusC)
				return "BDF+C";
			if (format == DataFormat::BdfPlusD)
				return "BDF+D";
			return "";
		}

		template <typename T>
		std::string to_string_decimal(const T& t)
		{
			std::string str{ std::to_string(t) };
			std::replace(str.begin(), str.end(), ',', '.');
			int offset{ 1 };
			if (str.find_last_not_of('0') == str.find('.'))
			{
				offset = 0;
			}
			str.erase(str.find_last_not_of('0') + offset, std::string::npos);
			return str;
		}

	}

}
