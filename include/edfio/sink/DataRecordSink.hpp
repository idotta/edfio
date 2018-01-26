//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "Sink.hpp"
#include "../core/Record.hpp"

#include <iostream>
#include <fstream>

namespace edfio
{

	class DataRecordSink : public Sink<Record<char>, Record<char>*, Record<char>&, std::ofstream, std::output_iterator_tag>
	{
	public:

		class iterator : public sink_type::iterator
		{
			size_type m_offset = -1; // Default is end
			DataRecordSink *m_context = nullptr;
		public:

			// Construction
			iterator() = default;

			iterator(DataRecordSink *context, size_type offset = -1)
				: m_offset(offset)
				, m_context(context)
			{
			}

			iterator(const iterator &it)
				: m_offset(it.m_offset)
				, m_context(it.m_context)
			{
			}

			// Assignment
			iterator& operator=(value_type value)
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				m_context->save(m_offset, std::move(value));
				return *this;
			}

			// Equality
			bool operator==(const iterator &it) const
			{
				return (m_offset == it.m_offset && m_context == it.m_context);
			}
			bool operator!=(const iterator &it) const
			{
				return !(*this == it);
			}

			// Relation
			bool operator<(const iterator &it) const
			{
				if (m_context != it.m_context)
					throw std::invalid_argument("Iterators incompatible");
				return (m_offset < it.m_offset);
			}
			bool operator>(const iterator &it) const
			{
				if (m_context != it.m_context)
					throw std::invalid_argument("Iterators incompatible");
				return (m_offset > it.m_offset);
			}
			bool operator<=(const iterator &it) const
			{
				if (m_context != it.m_context)
					throw std::invalid_argument("Iterators incompatible");
				return (m_offset <= it.m_offset);
			}
			bool operator>=(const iterator &it) const
			{
				if (m_context != it.m_context)
					throw std::invalid_argument("Iterators incompatible");
				return (m_offset >= it.m_offset);
			}

			// Pre-increment
			iterator& operator++()
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (m_offset == -1)
					throw std::length_error("Iterator not incrementable");
				if (++m_offset >= m_context->size())
					m_offset = -1;
				return *this;
			}
			// Post-increment
			iterator operator++(int)
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				++*this;
				return tmp;
			}
			// Pre-decrement
			iterator& operator--()
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (m_offset == 0)
					throw std::length_error("Iterator not decrementable");
				if (m_offset == -1)
					m_offset = m_context->size() - 1;
				else
					m_offset--;
				return *this;
			}
			// Post-decrement
			iterator operator--(int)
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				--*this;
				return tmp;
			}
			// Compound addition assignment
			iterator& operator+=(size_type off)
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (m_offset == -1)
					throw std::length_error("Iterator not incrementable");
				if (m_offset + off > m_context->size())
					throw std::length_error("Iterator + offset out of range");
				if (m_offset + off == m_context->size())
					m_offset = -1;
				else
					m_offset += off;
				return *this;
			}
			// Addition
			iterator operator+(size_type off) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				tmp += off;
				return tmp;
			}
			// Compound subtraction assignment
			iterator& operator-=(size_type off)
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (m_offset == 0)
					throw std::length_error("Iterator not decrementable");
				if (m_offset != -1 && m_offset < off)
					throw std::length_error("Iterator - offset out of range");
				if (m_offset == -1)
					m_offset = m_context->size() - off;
				else
					m_offset -= off;
				return *this;
			}
			// Subtraction
			iterator operator-(size_type off) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				tmp -= off;
				return tmp;
			}
			difference_type operator-(iterator it) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (m_context != it.m_context)
					throw std::invalid_argument("Iterators incompatible");
				return difference_type(it.m_offset - m_offset);
			}

			// Dereference
			iterator& operator*()
			{
				return *this;
			}
			iterator* operator->()
			{
				return this;
			}

			// Subscripting
			iterator& operator[](size_type)
			{
				return *this;
			}
		};

		typedef iterator const const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		DataRecordSink() = delete;

		DataRecordSink(stream_type &stream, size_type recordSize, size_type storeSize, std::streamoff headerOffset)
			: sink_type(stream)
			, m_recordSize(recordSize)
			, m_headerOffset(headerOffset)
			, m_value(recordSize)
		{
			measure();
		}

		iterator begin()
		{
			return iterator(this, 0);
		}
		const_iterator begin() const
		{
			return const_iterator(const_cast<DataRecordSink*>(this), 0);
		}
		const_iterator cbegin() const
		{
			return const_iterator(const_cast<DataRecordSink*>(this), 0);
		}
		iterator end()
		{
			return iterator(this);
		}
		const_iterator end() const
		{
			return const_iterator(const_cast<DataRecordSink*>(this));
		}
		const_iterator cend() const
		{
			return const_iterator(const_cast<DataRecordSink*>(this));
		}
		reverse_iterator rbegin()
		{
			return reverse_iterator(end());
		}
		const_reverse_iterator rbegin() const
		{
			return const_reverse_iterator(end());
		}
		const_reverse_iterator crbegin() const
		{
			return const_reverse_iterator(cend());
		}
		reverse_iterator rend()
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator rend() const
		{
			return const_reverse_iterator(begin());
		}
		const_reverse_iterator crend() const
		{
			return const_reverse_iterator(cbegin());
		}

		size_type size() const
		{
			return m_sinkSize;
		}

	protected:
		void measure()
		{
			auto pos = m_stream.tellp();
			m_stream.seekp(0, std::ios::end);
			size_type sz = m_stream.tellp();
			m_stream.seekp(pos);
			if (sz < m_headerOffset)
				throw std::invalid_argument("Invalid header");
			m_sinkSize = (sz - m_headerOffset) / m_recordSize;
		}

		void save(size_type off, value_type value)
		{
			measure();
			if (off >= m_sinkSize || off == -1)
			{
				m_stream.seekp(0, std::ios::end);
				m_sinkSize++;
			}
			else
			{
				off = m_headerOffset + off * m_recordSize;
				m_stream.seekp(off, std::ios::beg);
			}
			m_value() = std::move(value());
			m_stream << m_value;
		}

		size_type m_recordSize;
		size_type m_sinkSize;
		std::streamoff m_headerOffset;
		value_type m_value;
	};

}
