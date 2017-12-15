//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../core/Streamer.hpp"
#include "../core/Record.hpp"

namespace edfio
{

	struct ReaderRecord : Reader<RecordField>
	{
		ReaderRecord(size_t recordSize) : m_recordSize(recordSize) {}

		ResultT operator ()(StreamT &stream);
	private:
		size_t m_recordSize = 0;
	};

}

#include "impl/ReaderRecord.ipp"
