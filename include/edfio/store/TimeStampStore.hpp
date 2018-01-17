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

	class TimeStampStore : public RecordStore
	{
	public:

		typedef RecordStore::iterator iterator;
		typedef iterator const const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator; //optional
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator; //optional

		TimeStampStore() = delete;

		TimeStampStore(stream_type &stream, size_type recordSize, size_type storeSize,
			std::streamoff headerOffset, size_type datarecordSize, std::streamoff signalOffset)
			: RecordStore(stream, recordSize, storeSize, headerOffset)
			, m_datarecordSize(datarecordSize)
			, m_signalOffset(signalOffset)
		{
		}

	protected:

		void load(size_type off) override
		{
			if (off < 0 || off >= size())
			{
				throw std::out_of_range("Iterator not dereferenceable");
			}

			if (!m_stream.good())
				m_stream.clear();

			auto currentPos = m_stream.tellg();
			auto destPos = m_headerOffset + m_datarecordSize * off + m_signalOffset;
			if (destPos != currentPos)
			{
				m_stream.seekg(destPos, std::ios::beg);
			}
			m_stream.getline(m_value().data(), m_datarecordSize, 20);
		}

		size_type m_datarecordSize;
		std::streamoff m_signalOffset;
	};

}
