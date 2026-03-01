//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "..\Config.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace edfio {
template <ProcessorErrorCheck Check, typename CharT>
inline bool CheckFormatErrors(const std::basic_string<CharT> &str) {
  if constexpr (Check == ProcessorErrorCheck::Permissive) {
    return false;
  } else {
    for (auto c : str) {
      if (!std::isprint(static_cast<unsigned char>(c)))
        return true;
    }
    return false;
  }
}

template <ProcessorErrorCheck Check, typename CharT>
inline bool CheckFormatErrors(const std::vector<CharT> &str) {
  if constexpr (Check == ProcessorErrorCheck::Permissive) {
    return false;
  } else {
    for (auto c : str) {
      if (!std::isprint(static_cast<unsigned char>(c)))
        return true;
    }
    return false;
  }
}

namespace detail {

inline constexpr char ADDITIONAL_SEPARATOR = '|';
inline constexpr std::array<std::string_view, 12> MONTHS = {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

template <typename CharT>
inline bool CheckFormatErrors(const std::basic_string<CharT> &str) {
  return edfio::CheckFormatErrors<PROCESSOR_ERROR_CHECKING, CharT>(str);
}

template <typename CharT>
inline bool CheckFormatErrors(const std::vector<CharT> &str) {
  return edfio::CheckFormatErrors<PROCESSOR_ERROR_CHECKING, CharT>(str);
}

inline int32_t GetMonthFromString(std::string_view str) {
  for (size_t idx = 0; idx < MONTHS.size(); idx++) {
    if (str == MONTHS[idx]) {
      return idx + 1;
    }
  }
  return 0;
}

inline std::string GetStringFromMonth(size_t idx) {
  if (idx >= 1 && idx <= 12)
    return std::string{MONTHS[idx - 1]};
  return "JAN";
}

inline std::string ReduceString(std::string_view value) {
  // Trim leading spaces
  auto start = value.find_first_not_of(' ');
  if (start == std::string::npos)
    return "";
  // Trim trailing spaces
  auto end = value.find_last_not_of(' ');
  // Collapse interior runs of spaces to single space
  std::string result;
  result.reserve(end - start + 1);
  bool prev_space = false;
  for (size_t i = start; i <= end; ++i) {
    if (value[i] == ' ') {
      if (!prev_space) {
        result += ' ';
        prev_space = true;
      }
    } else {
      result += value[i];
      prev_space = false;
    }
  }
  return result;
}

inline std::string_view GetFormatName(DataFormat format) {
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

inline int32_t ParseInt(std::string_view sv, const char *error_msg) {
  while (!sv.empty() && sv.front() == ' ')
    sv.remove_prefix(1);
  while (!sv.empty() && sv.back() == ' ')
    sv.remove_suffix(1);
  int32_t value{};
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
  if (ec != std::errc{})
    throw std::invalid_argument(error_msg);
  return value;
}

inline int64_t ParseLongLong(std::string_view sv, const char *error_msg) {
  while (!sv.empty() && sv.front() == ' ')
    sv.remove_prefix(1);
  while (!sv.empty() && sv.back() == ' ')
    sv.remove_suffix(1);
  int64_t value{};
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
  if (ec != std::errc{})
    throw std::invalid_argument(error_msg);
  return value;
}

inline double ParseDouble(std::string_view sv, const char *error_msg) {
  while (!sv.empty() && sv.front() == ' ')
    sv.remove_prefix(1);
  while (!sv.empty() && sv.back() == ' ')
    sv.remove_suffix(1);
  // std::from_chars accepts '-' but not '+' by standard; strip leading '+'
  if (!sv.empty() && sv.front() == '+')
    sv.remove_prefix(1);
  double value{};
  auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
  if (ec != std::errc{})
    throw std::invalid_argument(error_msg);
  return value;
}

template <typename T> inline std::string to_string_decimal(const T &t) {
  std::string str{std::to_string(t)};
  std::replace(str.begin(), str.end(), ',', '.');
  int32_t offset{1};
  if (str.find_last_not_of('0') == str.find('.')) {
    offset = 0;
  }
  str.erase(str.find_last_not_of('0') + offset, std::string::npos);
  return str;
}
} // namespace detail

} // namespace edfio
