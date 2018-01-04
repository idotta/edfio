//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../DataRecordStore.hpp"
#include "../SignalRecordStore.hpp"
#include "../SignalSampleStore.hpp"

#include "../../header/HeaderGeneral.hpp"
#include "../../header/HeaderSignal.hpp"

namespace edfio
{

	namespace detail
	{

		template <class Stream>
		DataRecordStore CreateDataRecordStore(Stream &stream, const HeaderGeneral &general)
		{
			auto recordSize = general.m_detail.m_recordSize;
			auto storeSize = general.m_datarecordsFile;
			auto headerSize = general.m_headerSize;
			return DataRecordStore(stream, recordSize, storeSize, headerSize);
		}

		template <class Stream>
		SignalRecordStore CreateSignalRecordStore(Stream &stream, const HeaderGeneral &general, const HeaderSignal &signal)
		{
			auto recordSize = general.m_detail.m_recordSize;
			auto storeSize = general.m_datarecordsFile;
			auto headerSize = general.m_headerSize;
			auto signalSize = signal.m_samplesInDataRecord * GetSampleBytes(general.m_version);
			auto signalOff = signal.m_detail.m_bufferOffset;
			return SignalRecordStore(stream, signalSize, storeSize, headerSize, recordSize, signalOff);
		}

		template <class Stream>
		SignalSampleStore CreateSignalSampleStore(Stream &stream, const HeaderGeneral &general, const HeaderSignal &signal)
		{
			auto recordSize = general.m_detail.m_recordSize;
			auto storeSize = general.m_datarecordsFile * signal.m_samplesInDataRecord;
			auto headerSize = general.m_headerSize;
			auto sampleSize = GetSampleBytes(general.m_version);
			auto signalSize = signal.m_samplesInDataRecord * sampleSize;
			auto signalOff = signal.m_detail.m_bufferOffset;
			return SignalSampleStore(stream, sampleSize, storeSize, headerSize, recordSize, signalSize, signalOff);
		}

	}

}