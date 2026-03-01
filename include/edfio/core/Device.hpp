//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include <cstdint>
#include <iterator>

namespace edfio {

// A class created in order to have an easier way to access streams
// of specific data through their respective iterators.
template <class Value, class Pointer, class Reference, class Stream,
          typename IterCategory>
class Device {
public:
  using stream_type = Stream;
  using device_type = Device<Value, Pointer, Reference, Stream, IterCategory>;

  using value_type = Value;
  using pointer = Pointer;
  using reference = Reference;
  using difference_type = int64_t;
  using size_type = uint64_t;

  class iterator {
  public:
    using difference_type = typename Device::difference_type;
    using value_type = typename Device::value_type;
    using reference = typename Device::reference;
    using pointer = typename Device::pointer;
    using iterator_category = IterCategory;
    using iterator_concept = IterCategory;
    using stream_type = typename Device::stream_type;
  };

  Device() = delete;

  Device(stream_type &stream) : m_stream(stream) {}

protected:
  stream_type &m_stream;
};

} // namespace edfio
