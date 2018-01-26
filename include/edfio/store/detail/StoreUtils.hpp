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
#include "../TimeStampStore.hpp"
#include "../../header/HeaderGeneral.hpp"
#include "../../header/HeaderSignal.hpp"

#include <utility>

namespace edfio
{

	namespace detail
	{

		template <class Stream>
		DataRecordStore CreateDataRecordStore(Stream &stream, const HeaderGeneral &general)
		{
			DataRecordStore::size_type recordSize = general.m_detail.m_recordSize;
			DataRecordStore::size_type storeSize = general.m_datarecordsFile;
			std::streamoff headerSize = general.m_headerSize;
			return std::move(DataRecordStore{ stream, recordSize, storeSize, headerSize });
		}

		template <class Stream>
		SignalRecordStore CreateSignalRecordStore(Stream &stream, const HeaderGeneral &general, const HeaderSignal &signal)
		{
			SignalRecordStore::size_type recordSize = general.m_detail.m_recordSize;
			SignalRecordStore::size_type storeSize = general.m_datarecordsFile;
			std::streamoff headerSize = general.m_headerSize;
			SignalRecordStore::size_type signalSize = signal.m_samplesInDataRecord * GetSampleBytes(general.m_version);
			std::streamoff signalOff = signal.m_detail.m_bufferOffset;
			return std::move(SignalRecordStore{ stream, signalSize, storeSize, headerSize, recordSize, signalOff });
		}

		template <class Stream>
		SignalSampleStore CreateSignalSampleStore(Stream &stream, const HeaderGeneral &general, const HeaderSignal &signal)
		{
			SignalSampleStore::size_type recordSize = general.m_detail.m_recordSize;
			SignalSampleStore::size_type storeSize = general.m_datarecordsFile * signal.m_samplesInDataRecord;
			std::streamoff headerSize = general.m_headerSize;
			SignalSampleStore::size_type sampleSize = GetSampleBytes(general.m_version);
			SignalSampleStore::size_type signalSize = signal.m_samplesInDataRecord;
			std::streamoff signalOff = signal.m_detail.m_bufferOffset;
			return std::move(SignalSampleStore{ stream, sampleSize, storeSize, headerSize, recordSize, signalSize, signalOff });
		}

		template <class Stream>
		TimeStampStore CreateTimeStampStore(Stream &stream, const HeaderGeneral &general, const HeaderSignal &signal)
		{
			TimeStampStore::size_type recordSize = general.m_detail.m_recordSize;
			TimeStampStore::size_type storeSize = general.m_datarecordsFile;
			std::streamoff headerSize = general.m_headerSize;
			TimeStampStore::size_type signalSize = signal.m_samplesInDataRecord * GetSampleBytes(general.m_version);
			std::streamoff signalOff = signal.m_detail.m_bufferOffset;
			return std::move(TimeStampStore{ stream, signalSize, storeSize, headerSize, recordSize, signalOff });
		}

	}

}