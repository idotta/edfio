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
#include <iterator>

// TAL - Timestamped Annotation List
// TalStore is a particular kind of Store which iterates 
// through a SignalRecordStore corresponding to an Annotation signal
// and dereferences a TAL

namespace edfio
{

	class TalStore : public Store<Record<char>::VectorType, const Record<char>, std::bidirectional_iterator_tag>
	{
	public:

		class iterator : public store_type::iterator
		{
			friend class TalStore;
			size_type m_offset = 0; // Absolute position in current Store
			TalStore *m_context = nullptr;
		public:

			// Construction
			iterator() = default;

		protected:
			iterator(TalStore *context, size_type off)
				: m_context(context)
				, m_offset(off)
			{
				if (m_offset == 0)
					++*this;
			}

		public:
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

				m_offset = m_context->next(m_offset);
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

				m_offset = m_context->prev(m_offset);
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
				return m_context->getR();
			}
			pointer operator->() const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				return m_context->getP();
			}
		};

		typedef iterator const const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		TalStore() = delete;

		TalStore(stream_type &stream)
			: store_type(stream)
		{
		}

		iterator begin()
		{
			return iterator(this, 0);
		}
		const_iterator begin() const
		{
			return const_iterator(const_cast<TalStore*>(this), 0);
		}
		const_iterator cbegin() const
		{
			return const_iterator(const_cast<TalStore*>(this), 0);
		}
		iterator end()
		{
			return iterator(this, m_stream.Size());
		}
		const_iterator end() const
		{
			return const_iterator(const_cast<TalStore*>(this), m_stream.Size());
		}
		const_iterator cend() const
		{
			return const_iterator(const_cast<TalStore*>(this), m_stream.Size());
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
		reference getR()
		{
			return m_value;
		}

		pointer getP()
		{
			return &m_value;
		}

		size_type next(size_type off)
		{
			if (off >= m_stream().size())
				throw std::length_error("Iterator not incrementable");

			if (off < m_stream().size())
			{
				auto first = m_stream().begin() + off;
				auto last = m_stream().end();

				while (first != last && *first == 0)
				{
					first++;
					off++;
				}

				if (first != last)
				{
					size_type offOld = off;
					for (auto it = first; *it != 0 && it != last; it++)
					{
						off++;
					}
					if (offOld != off)
					{
						m_value.assign(first, first + (off - offOld));
					}
				}
			}
			return off;
		}

		size_type prev(size_type off)
		{
			if (off <= 0)
				throw std::length_error("Iterator not decrementable");

			if (off > 0)
			{
				auto first = m_stream().rend() + off;
				auto last = m_stream().rend();

				while (*first == 0 && first != last)
				{
					first++;
					off--;
				}

				if (first != last)
				{
					size_type offOld = off;
					for (auto it = first; *it != 0 && it != last; it++)
					{
						off--;
					}
					if (offOld != off)
					{
						m_value.assign(first + offOld, first + off);
					}
				}
			}
			return off;
		}

		value_type m_value;
	};

}
