//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Errors.hpp"
#include "../core/Annotation.hpp"
#include "../core/Record.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>

namespace edfio {

inline TimeStamp ProcessTimeStampRecord(Record<char> record,
                                        int64_t datarecord) {
  TimeStamp timestamp;
  timestamp.m_datarecord = datarecord;
  auto &value = record();

  // TimeStamp MUST start with '+' or '-'
  if (value.front() != '+' && value.front() != '-') {
    throw std::invalid_argument(
        GetError(FileErrc::FileContainsInvalidAnnotations));
  }

  // Make sure it's a valid timestamp
  static constexpr std::array<char, 2> comp = {detail::ANNOTATION_END,
                                                detail::ANNOTATION_DIV};
  auto result = std::ranges::find_first_of(value, comp);

  if (result == value.end()) {
    throw std::invalid_argument(
        GetError(FileErrc::FileContainsInvalidAnnotations));
  } else {
    *result = 0;
    double start{};
    auto [ptr, ec] =
        std::from_chars(value.data(), value.data() + value.size(), start);
    if (ec != std::errc{} || ptr == value.data()) {
      throw std::invalid_argument(
          GetError(FileErrc::FileContainsInvalidAnnotations));
    }
    timestamp.m_start = start;
  }
  return timestamp;
}

} // namespace edfio
