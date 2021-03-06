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

	// A class created in order to have an easier way to read streams
	// of specific data through their respective iterators.
	template <class Value, class Pointer, class Reference, class Stream, typename IterCategory>
	class Store : public Device<Value, Pointer, Reference, Stream, IterCategory>
	{
	public:
		typedef Store<Value, Pointer, Reference, Stream, IterCategory> store_type;
		typedef device_type::iterator iterator;
		typedef iterator const const_iterator;

		Store() = delete;

		Store(stream_type &stream)
			: device_type(stream)
		{
		}
	};

}
