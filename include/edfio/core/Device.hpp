//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include <iterator>

namespace edfio
{

	// A class created in order to have an easier way to access streams
	// of specific data through their respective iterators.
	template <class Value, class Pointer, class Reference, class Stream, typename IterCategory>
	class Device
	{
	public:
		typedef Stream stream_type;
		typedef Device<Value, Pointer, Reference, Stream, IterCategory> device_type;

		typedef Value value_type;
		typedef Pointer pointer;
		typedef Reference reference;
		typedef long long difference_type;
		typedef unsigned long long size_type;

		class iterator
		{
		public:
			typedef typename Device::difference_type difference_type;
			typedef typename Device::value_type value_type;
			typedef typename Device::reference reference;
			typedef typename Device::pointer pointer;
			typedef IterCategory iterator_category;
			typedef typename Device::stream_type stream_type;
		};

		Device() = delete;

		Device(stream_type &stream)
			: m_stream(stream)
		{
		}

	protected:

		stream_type &m_stream;
	};

}
