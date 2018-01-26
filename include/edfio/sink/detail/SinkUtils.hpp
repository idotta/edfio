//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../DataRecordSink.hpp"
#include "../../header/HeaderGeneral.hpp"
#include "../../header/HeaderSignal.hpp"

#include <utility>

namespace edfio
{

	namespace detail
	{

		template <class Stream>
		DataRecordSink CreateDataRecordSink(Stream &stream, const HeaderGeneral &general)
		{
			DataRecordStore::size_type recordSize = general.m_detail.m_recordSize;
			DataRecordStore::size_type sinkSize = general.m_datarecordsFile;
			std::streamoff headerSize = general.m_headerSize;
			return std::move(DataRecordSink{ stream, recordSize, sinkSize, headerSize });
		}

	}

}