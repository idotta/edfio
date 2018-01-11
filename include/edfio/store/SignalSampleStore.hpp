//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "RecordStore.hpp"

namespace edfio
{

	class SignalSampleStore : public RecordStore
	{
	public:

		typedef RecordStore::iterator iterator;
		typedef iterator const const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator; //optional
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator; //optional

		SignalSampleStore() = delete;

		SignalSampleStore(stream_type &stream, size_type recordSize, size_type storeSize,
			std::streamoff headerOffset, size_type datarecordSize,
			size_type signalrecordSize, std::streamoff signalOffset)
			: RecordStore(stream, recordSize, storeSize, headerOffset)
			, m_datarecordSize(datarecordSize)
			, m_signalrecordSize(signalrecordSize)
			, m_signalOffset(signalOffset)
			, m_buffer(recordSize * signalrecordSize)
			, m_bufferPos(-1)
		{
		}

	protected:

		void load(size_type off) override
		{
			if (off < 0 || off >= size())
			{
				throw std::out_of_range("Iterator not dereferenceable");
			}

			auto dataRecordOffset = off / m_signalrecordSize;
			auto sampleOffset = off % m_signalrecordSize;
			auto destPos = m_headerOffset + dataRecordOffset * m_datarecordSize + m_signalOffset + sampleOffset * m_recordSize;

			if (m_bufferPos < 0 || (destPos < m_bufferPos || destPos >= m_bufferPos + m_buffer.Size()))
			{
				readStream(destPos);
				m_bufferPos = m_headerOffset + dataRecordOffset * m_datarecordSize + m_signalOffset;
			}

			auto first = m_buffer().begin() + sampleOffset * m_recordSize;
			std::copy(first, first + m_value.Size(), m_value().begin());
		}

		void readStream(long long newPos)
		{
			if (!m_stream.good())
				m_stream.clear();

			auto oldPos = m_stream.tellg();
			if (newPos != oldPos)
			{
				m_stream.seekg(newPos, std::ios::beg);
			}
			m_stream >> m_buffer;
		}

		size_type m_datarecordSize;
		size_type m_signalrecordSize;
		std::streamoff m_signalOffset;
		// Samples buffer to decrease reading time
		value_type m_buffer;
		std::streamoff m_bufferPos;
	};

}
