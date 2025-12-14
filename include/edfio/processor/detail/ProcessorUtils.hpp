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
#include <string_view>
#include <vector>
#include <cctype>
#include <regex>
#include <concepts>

namespace edfio
{

	namespace impl
	{

		template <ProcessorErrorCheck Check, typename CharT>
			requires (Check == ProcessorErrorCheck::Strict)
		static bool CheckFormatErrors(const std::basic_string<CharT> &str)
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
			requires (Check == ProcessorErrorCheck::Permissive)
		static bool CheckFormatErrors(const std::basic_string<CharT> &str)
		{
			return false;
		}

		template <ProcessorErrorCheck Check, typename CharT>
			requires (Check == ProcessorErrorCheck::Strict)
		static bool CheckFormatErrors(const std::vector<CharT> &str)
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
			requires (Check == ProcessorErrorCheck::Permissive)
		static bool CheckFormatErrors(const std::vector<CharT> &str)
		{
			return false;
		}

	}

	namespace detail
	{

		inline constexpr char ADDITIONAL_SEPARATOR = '|';

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

		[[nodiscard]] constexpr int GetMonthFromString(std::string_view str)
		{
			constexpr std::string_view months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
			for (size_t idx = 0; idx < 12; idx++)
			{
				if (str == months[idx])
				{
					return static_cast<int>(idx + 1);
				}
			}
			return 0;
		}

		[[nodiscard]] constexpr std::string_view GetStringFromMonth(size_t idx)
		{
			constexpr std::string_view months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
			if (idx > 0 && idx <= 12)
				return months[idx - 1];
			return "JAN";
		}

		static std::string ReduceString(const std::string &value)
		{
			return std::regex_replace(value, std::regex("^ +| +$|( ) +"), "$1");
		}

		[[nodiscard]] constexpr const char* GetFormatName(DataFormat format)
		{
			switch (format)
			{
			case DataFormat::Edf:
				return "EDF";
			case DataFormat::EdfPlusC:
				return "EDF+C";
			case DataFormat::EdfPlusD:
				return "EDF+D";
			case DataFormat::Bdf:
				return "BDF";
			case DataFormat::BdfPlusC:
				return "BDF+C";
			case DataFormat::BdfPlusD:
				return "BDF+D";
			default:
				return "";
			}
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
