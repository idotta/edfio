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

#include <iterator>

namespace edfio
{

	// TAL - Timestamped Annotation List
	// TalStore is a particular kind of Store which iterates 
	// through a SignalRecordStore corresponding to an Annotation signal
	// and dereferences a TAL
	class TalStore : public Store<Record<char>::VectorType, Record<char>::VectorType const*, Record<char>::VectorType const&, const Record<char>, std::bidirectional_iterator_tag>
	{
		using base_store = Store<Record<char>::VectorType, Record<char>::VectorType const*, Record<char>::VectorType const&, const Record<char>, std::bidirectional_iterator_tag>;
	public:
		using typename base_store::stream_type;
		using typename base_store::value_type;
		using typename base_store::pointer;
		using typename base_store::reference;
		using typename base_store::difference_type;
		using typename base_store::size_type;

		class iterator : public base_store::iterator
		{
			friend class TalStore;
			size_type m_offset = 0; // Absolute position in current Store
			const TalStore *m_context = nullptr;
		public:

			// Construction
			iterator() = default;

		protected:
			iterator(const TalStore *context, size_type off)
				: m_offset(off)
				, m_context(context)
			{
				if (m_offset == 0)
					++*this;
			}

		public:
			iterator(const iterator &it)
				: m_offset(it.m_offset)
				, m_context(it.m_context)
			{
			}

			// Assignment
			iterator& operator=(const iterator &it)
			{
				m_context = it.m_context;
				m_offset = it.m_offset;
				return *this;
			}

			// Equality (!= auto-generated)
			bool operator==(const iterator &it) const
			{
				return (m_offset == it.m_offset && m_context == it.m_context);
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

		using const_iterator = iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		TalStore() = delete;

		TalStore(stream_type &stream)
			: store_type(stream)
		{
		}

		iterator begin() const
		{
			return iterator(this, 0);
		}
		const_iterator cbegin() const
		{
			return begin();
		}
		iterator end() const
		{
			return iterator(this, m_stream.Size());
		}
		const_iterator cend() const
		{
			return end();
		}
		reverse_iterator rbegin() const
		{
			return reverse_iterator(end());
		}
		const_reverse_iterator crbegin() const
		{
			return const_reverse_iterator(cend());
		}
		reverse_iterator rend() const
		{
			return reverse_iterator(begin());
		}
		const_reverse_iterator crend() const
		{
			return const_reverse_iterator(cbegin());
		}

	protected:
		reference getR() const
		{
			return m_value;
		}

		pointer getP() const
		{
			return &m_value;
		}

		size_type next(size_type off) const
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

		size_type prev(size_type off) const
		{
			if (off == 0)
				throw std::length_error("Iterator not decrementable");

			auto const& data = m_stream();

			// Walk backward from position (off - 1), skipping zeros
			size_type pos = off;
			while (pos > 0 && data[pos - 1] == 0)
				--pos;

			if (pos == 0)
				throw std::length_error("Iterator not decrementable");

			// Find the start of the previous non-zero TAL
			size_type end_of_tal = pos;
			while (pos > 0 && data[pos - 1] != 0)
				--pos;

			m_value.assign(data.begin() + pos, data.begin() + end_of_tal);
			return pos;
		}

		mutable value_type m_value;
	};

}
