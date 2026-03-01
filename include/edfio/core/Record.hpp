//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include <istream>
#include <ostream>
#include <ranges>
#include <vector>

namespace edfio {

// Records have fixed sizes that can vary according to the signal
// They can be used for either single signal record or data record IO
// It is important that data records always have one size in the same file
template <typename ValT = char> struct Record {
  using ValueType = ValT;
  using VectorType = std::vector<ValueType>;

  Record() = delete;

  explicit Record(typename VectorType::size_type recordSize)
      : m_value(recordSize, 0) {}

  Record(typename VectorType::const_iterator first,
         typename VectorType::const_iterator last)
      : m_value(first, last) {}

  Record(const Record &) = default;
  Record(Record &&) = default;
  Record &operator=(const Record &) = default;
  Record &operator=(Record &&) = default;

  [[nodiscard]] typename VectorType::size_type Size() const {
    return m_value.size();
  }
  [[nodiscard]] const VectorType &operator()() const { return m_value; }
  VectorType &operator()() { return m_value; }
  Record<ValueType> operator+(const Record<ValueType> &record) const {
    Record<ValueType> tmp(Size() + record.Size());
    std::ranges::copy(m_value, tmp().begin());
    std::ranges::copy(record(), tmp().begin() + Size());
    return tmp;
  }

  VectorType m_value;
};

template <typename ValT = char>
std::ostream &operator<<(std::ostream &os, const Record<ValT> &r) {
  const auto &record = r();
  os.write(record.data(), r.Size() * sizeof(ValT));
  return os;
}

template <typename ValT = char>
std::istream &operator>>(std::istream &is, Record<ValT> &r) {
  auto &record = r();
  record.resize(r.Size(), 0);
  is.read(&record[0], r.Size() * sizeof(ValT));
  return is;
}

} // namespace edfio
