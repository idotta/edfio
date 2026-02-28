//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/Device.hpp"

namespace edfio
{

	// A class created in order to have an easier way to write streams
	// of specific data through their respective iterators.
	template <class Value, class Pointer, class Reference, class Stream, typename IterCategory>
	class Sink : public Device<Value, Pointer, Reference, Stream, IterCategory>
	{
	public:
		using device_type = Device<Value, Pointer, Reference, Stream, IterCategory>;
		using sink_type = Sink<Value, Pointer, Reference, Stream, IterCategory>;
		using typename device_type::stream_type;
		using typename device_type::value_type;
		using typename device_type::pointer;
		using typename device_type::reference;
		using typename device_type::difference_type;
		using typename device_type::size_type;
		using iterator = typename device_type::iterator;

		Sink() = delete;

		Sink(stream_type &stream)
			: device_type(stream)
		{
		}
	};

}
