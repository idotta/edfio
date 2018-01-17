//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "Store.hpp"
#include "../core/Record.hpp"

#include <iostream>
#include <fstream>
#include <iterator>

namespace edfio
{

	class RecordStore : public Store<Record<char>, std::ifstream, std::random_access_iterator_tag>
	{
	public:

		class iterator : public store_type::iterator
		{
			size_type m_offset = 0; // Relative to total of Stores
			RecordStore *m_context = nullptr;
		public:

			// Construction
			iterator() = default;

			iterator(RecordStore *context, size_type offset = 0)
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
			iterator& operator=(const iterator &it)
			{
				m_offset = it.m_offset;
				m_context = it.m_context;
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
				if (m_context->size() <= 0 || m_offset + 1 > m_context->size())
					throw std::length_error("Iterator not incrementable");
				m_offset++;
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
				if (m_context->size() <= 0 || m_offset - 1 < 0)
					throw std::length_error("Iterator not decrementable");
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
				if (m_context->size() <= 0 || m_offset + off > m_context->size())
					throw std::length_error("Iterator + offset out of range");
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
				if (m_context->size() <= 0 || m_offset - off < 0)
					throw std::length_error("Iterator - offset out of range");
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
			reference operator*() const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				return m_context->getR(m_offset);
			}
			pointer operator->() const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				return m_context->getP(m_offset);
			}

			// Subscripting
			reference operator[](size_type off) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				tmp += off;
				return *tmp;
			}
		};

		typedef iterator const const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		RecordStore() = delete;

		RecordStore(stream_type &stream, size_type recordSize, size_type storeSize, std::streamoff headerOffset)
			: store_type(stream)
			, m_recordSize(recordSize)
			, m_storeSize(storeSize)
			, m_headerOffset(headerOffset)
			, m_value(recordSize)
		{
		}

		iterator begin()
		{
			return iterator(this);
		}
		const_iterator begin() const
		{
			return const_iterator(const_cast<RecordStore*>(this));
		}
		const_iterator cbegin() const
		{
			return const_iterator(const_cast<RecordStore*>(this));
		}
		iterator end()
		{
			return iterator(this, size());
		}
		const_iterator end() const
		{
			return const_iterator(const_cast<RecordStore*>(this), size());
		}
		const_iterator cend() const
		{
			return const_iterator(const_cast<RecordStore*>(this), size());
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

		// Overrides
		virtual size_type size() const
		{
			return m_storeSize;
		}

	protected:
		virtual reference getR(size_type off)
		{
			load(off);
			return m_value;
		}

		virtual pointer getP(size_type off)
		{
			load(off);
			return &m_value;
		}

		virtual void load(size_type off) = 0;

		size_type m_recordSize;
		size_type m_storeSize;
		std::streamoff m_headerOffset;
		value_type m_value;
	};

}
