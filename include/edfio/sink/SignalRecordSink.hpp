//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "RecordSink.hpp"

namespace edfio
{

	class SignalRecordSink : public RecordSink
	{
	public:

		typedef RecordSink::iterator iterator;
		typedef iterator const const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		SignalRecordSink() = delete;

		SignalRecordSink(stream_type &stream, size_type recordSize, size_type storeSize, std::streamoff headerOffset, size_type datarecordSize, std::streamoff signalOffset)
			: RecordSink(stream, recordSize, storeSize, headerOffset)
			, m_datarecordSize(datarecordSize)
			, m_signalOffset(signalOffset)
		{
			measure();
		}

	protected:
		void measure() override
		{
			auto pos = m_stream.tellp();
			m_stream.seekp(0, std::ios::end);
			size_type sz = m_stream.tellp();
			m_stream.seekp(pos);
			if (sz < m_headerOffset)
				throw std::invalid_argument("Invalid header");
			
			size_type datarecords = (sz - m_headerOffset) / m_datarecordSize;
			size_type offset = (sz - m_headerOffset) % m_datarecordSize;;
			
			m_sinkSize = datarecords;
			if (offset >= m_signalOffset)
				m_sinkSize++;
		}

		void save(size_type off, value_type value) override
		{
			measure();
			if (off >= m_sinkSize || off == -1)
			{
				m_stream.seekp(0, std::ios::end);
				size_type sz = m_stream.tellp();
				size_type offset = (sz - m_headerOffset) % m_datarecordSize;;
				if (offset < m_signalOffset)
					throw std::invalid_argument("Invalid signal order");
				m_sinkSize++;
			}
			else
			{
				off = m_headerOffset + off * m_datarecordSize + m_signalOffset;
				m_stream.seekp(off, std::ios::beg);
			}
			m_value() = std::move(value());
			m_stream << m_value;
		}

		size_type m_datarecordSize;
		std::streamoff m_signalOffset;
	};

}
