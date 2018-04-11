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
#include "../SignalRecordSink.hpp"
#include "../../header/HeaderGeneral.hpp"
#include "../../header/HeaderSignal.hpp"

#include <utility>

namespace edfio
{

	namespace detail
	{

		template <class Stream>
		static DataRecordSink CreateDataRecordSink(Stream &stream, const HeaderGeneral &general)
		{
			DataRecordSink::size_type recordSize = general.m_detail.m_recordSize;
			DataRecordSink::size_type sinkSize = general.m_datarecordsFile;
			std::streamoff headerSize = general.m_headerSize;
			return std::move(DataRecordSink{ stream, recordSize, sinkSize, headerSize });
		}

		template <class Stream>
		static SignalRecordSink CreateSignalRecordSink(Stream &stream, const HeaderGeneral &general, const HeaderSignal &signal)
		{
			SignalRecordSink::size_type recordSize = general.m_detail.m_recordSize;
			SignalRecordSink::size_type sinkSize = general.m_datarecordsFile;
			std::streamoff headerSize = general.m_headerSize;
			SignalRecordSink::size_type signalSize = signal.m_samplesInDataRecord * GetSampleBytes(general.m_version);
			std::streamoff signalOff = signal.m_detail.m_signalOffset;
			return std::move(SignalRecordSink{ stream, signalSize, sinkSize, headerSize, recordSize, signalOff });
		}

	}

}