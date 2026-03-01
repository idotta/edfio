//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/Record.hpp"
#include "Sink.hpp"


#include <compare>
#include <fstream>
#include <optional>


namespace edfio {

class RecordSink : public Sink<Record<char>, Record<char> *, Record<char> &,
                               std::ofstream, std::output_iterator_tag> {
  using base_sink = Sink<Record<char>, Record<char> *, Record<char> &,
                         std::ofstream, std::output_iterator_tag>;

public:
  using typename base_sink::difference_type;
  using typename base_sink::pointer;
  using typename base_sink::reference;
  using typename base_sink::size_type;
  using typename base_sink::stream_type;
  using typename base_sink::value_type;

  class iterator : public base_sink::iterator {
    std::optional<size_type> m_offset; // nullopt = end
    RecordSink *m_context = nullptr;

  public:
    // Construction
    iterator() = default;

    iterator(RecordSink *context,
             std::optional<size_type> offset = std::nullopt)
        : m_offset(offset), m_context(context) {}

    iterator(const iterator &it)
        : m_offset(it.m_offset), m_context(it.m_context) {}

    // Assignment
    iterator &operator=(value_type value) {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      if (!m_offset)
        throw std::length_error("Cannot assign to end iterator");
      m_context->save(*m_offset, std::move(value));
      return *this;
    }

    // Equality (!= auto-generated)
    bool operator==(const iterator &it) const {
      return (m_offset == it.m_offset && m_context == it.m_context);
    }

    // Three-way comparison (<, >, <=, >= auto-generated)
    std::strong_ordering operator<=>(const iterator &it) const {
      if (m_context != it.m_context)
        throw std::invalid_argument("Iterators incompatible");
      auto lhs = m_offset.value_or(m_context->size());
      auto rhs = it.m_offset.value_or(it.m_context->size());
      return lhs <=> rhs;
    }

    // Pre-increment
    iterator &operator++() {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      if (!m_offset)
        throw std::length_error("Iterator not incrementable");
      if (++(*m_offset) >= m_context->size())
        m_offset = std::nullopt;
      return *this;
    }
    // Post-increment
    iterator operator++(int) {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      iterator tmp = *this;
      ++*this;
      return tmp;
    }
    // Pre-decrement
    iterator &operator--() {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      if (m_offset && *m_offset == 0)
        throw std::length_error("Iterator not decrementable");
      if (!m_offset)
        m_offset = m_context->size() - 1;
      else
        (*m_offset)--;
      return *this;
    }
    // Post-decrement
    iterator operator--(int) {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      iterator tmp = *this;
      --*this;
      return tmp;
    }
    // Compound addition assignment
    iterator &operator+=(size_type off) {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      if (!m_offset)
        throw std::length_error("Iterator not incrementable");
      if (*m_offset + off > m_context->size())
        throw std::length_error("Iterator + offset out of range");
      if (*m_offset + off == m_context->size())
        m_offset = std::nullopt;
      else
        *m_offset += off;
      return *this;
    }
    // Addition
    iterator operator+(size_type off) const {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      iterator tmp = *this;
      tmp += off;
      return tmp;
    }
    // Compound subtraction assignment
    iterator &operator-=(size_type off) {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      auto pos = m_offset.value_or(m_context->size());
      if (pos < off)
        throw std::length_error("Iterator - offset out of range");
      m_offset = pos - off;
      return *this;
    }
    // Subtraction
    iterator operator-(size_type off) const {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      iterator tmp = *this;
      tmp -= off;
      return tmp;
    }
    difference_type operator-(iterator it) const {
      if (!m_context)
        throw std::invalid_argument("Invalid context");
      if (m_context != it.m_context)
        throw std::invalid_argument("Iterators incompatible");
      auto lhs =
          static_cast<difference_type>(m_offset.value_or(m_context->size()));
      auto rhs = static_cast<difference_type>(
          it.m_offset.value_or(it.m_context->size()));
      return lhs - rhs;
    }

    // Dereference
    iterator &operator*() { return *this; }
    iterator *operator->() { return this; }

    // Subscripting
    iterator &operator[](size_type) { return *this; }
  };

  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  RecordSink() = delete;

  RecordSink(stream_type &stream, size_type recordSize, size_type sinkSize,
             std::streamoff headerOffset)
      : sink_type(stream), m_recordSize(recordSize), m_sinkSize(sinkSize),
        m_headerOffset(headerOffset), m_value(recordSize) {}

  iterator begin() { return iterator(this, 0); }
  const_iterator begin() const {
    return const_iterator(const_cast<RecordSink *>(this), 0);
  }
  const_iterator cbegin() const {
    return const_iterator(const_cast<RecordSink *>(this), 0);
  }
  iterator end() { return iterator(this); }
  const_iterator end() const {
    return const_iterator(const_cast<RecordSink *>(this));
  }
  const_iterator cend() const {
    return const_iterator(const_cast<RecordSink *>(this));
  }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(cend());
  }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(cbegin());
  }

  [[nodiscard]] size_type size() const { return m_sinkSize; }

protected:
  virtual void measure() = 0;
  virtual void save(size_type off, value_type value) = 0;

  size_type m_recordSize;
  size_type m_sinkSize;
  std::streamoff m_headerOffset;
  value_type m_value;
};

} // namespace edfio
