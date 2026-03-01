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
#include "ProcessorUtils.hpp"

#include <ranges>

namespace edfio {

inline Record<char> ProcessAnnotation(Annotation annotation) {
  if (annotation.m_annotation.empty()) {
    throw std::invalid_argument(
        GetError(FileErrc::FileWriteInvalidAnnotations));
  }

  std::string timestamp = (annotation.m_start >= 0 ? "+" : "") +
                          detail::to_string_decimal(annotation.m_start);
  std::string duration;
  if (annotation.m_duration > 0)
    duration = detail::to_string_decimal(annotation.m_duration);

  size_t size = timestamp.size(); // timestamp
  if (!duration.empty()) {
    size++;                  // 21 div
    size += duration.size(); // duration
  }

  size++;                                 // 20 div
  size += annotation.m_annotation.size(); // annotation
  size += 2;                              // 20 div and 0

  Record<char> record(size);
  auto it = record().begin();

  // timestamp
  std::ranges::copy(timestamp, it);
  it += timestamp.size();

  if (!duration.empty()) {
    *it++ = 21; // 21 div
    // duration
    std::ranges::copy(duration, it);
    it += duration.size();
  }

  *it++ = 20; // 20 div
  // annotation
  std::ranges::copy(annotation.m_annotation, it);
  it += annotation.m_annotation.size();

  *it++ = 20; // 20 div
  *it++ = 0;  // 0 end

  return record;
}

} // namespace edfio
