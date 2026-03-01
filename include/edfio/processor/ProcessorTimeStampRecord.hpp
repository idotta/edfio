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
  static const std::vector<char> comp = {detail::ANNOTATION_END,
                                         detail::ANNOTATION_DIV};
  auto result =
      std::find_first_of(value.begin(), value.end(), comp.begin(), comp.end());

  if (result == value.end()) {
    throw std::invalid_argument(
        GetError(FileErrc::FileContainsInvalidAnnotations));
  } else {
    *result = 0;
    char *end;
    double start = std::strtod(value.data(), &end);
    // On error
    if (end == value.data()) {
      throw std::invalid_argument(
          GetError(FileErrc::FileContainsInvalidAnnotations));
    } else {
      timestamp.m_start = start;
    }
  }
  return timestamp;
}

} // namespace edfio
