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

	class AnnotationStore : public Store<Record<char>, std::ifstream, std::bidirectional_iterator_tag>
	{
	public:

		class iterator : public store_type::iterator
		{
			enum
			{
				_begin = 0,
				_end = -1
			};
			AnnotationStore *m_context = nullptr;
			difference_type m_offset = _end;
		public:

			// Construction
			iterator() = default;

			iterator(AnnotationStore *context)
				: m_context(context)
				, m_offset(_end)
			{
			}

			iterator(const iterator &it)
				: m_context(it.m_context)
				, m_offset(it.m_offset)
			{
			}

			// Assignment
			iterator& operator=(const iterator &it)
			{
				m_context = it.m_context;
				m_offset = it.m_offset;
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
		};

		typedef iterator const const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		AnnotationStore() = delete;

		AnnotationStore(stream_type &stream, size_type recordSize, std::streamoff headerOffset)
			: store_type(stream)
			, m_recordSize(recordSize)
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
			return const_iterator(const_cast<AnnotationStore*>(this));
		}
		const_iterator cbegin() const
		{
			return const_iterator(const_cast<AnnotationStore*>(this));
		}
		iterator end()
		{
			return iterator(this, size());
		}
		const_iterator end() const
		{
			return const_iterator(const_cast<AnnotationStore*>(this), size());
		}
		const_iterator cend() const
		{
			return const_iterator(const_cast<AnnotationStore*>(this), size());
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

	protected:
		// Overrides
		reference getR(size_type off) override
		{
			load(off);
			return m_value;
		}

		pointer getP(size_type off) override
		{
			load(off);
			return &m_value;
		}

		void load(size_type off)
		{
		}

		size_type m_recordSize;
		std::streamoff m_headerOffset;
		value_type m_value;
	};

}
