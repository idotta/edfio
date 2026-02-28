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
#include <compare>

namespace edfio
{

	class RecordStore : public Store<Record<char>, Record<char> const*, Record<char> const&, std::ifstream, std::random_access_iterator_tag>
	{
		using base_store = Store<Record<char>, Record<char> const*, Record<char> const&, std::ifstream, std::random_access_iterator_tag>;
	public:
		using typename base_store::stream_type;
		using typename base_store::value_type;
		using typename base_store::pointer;
		using typename base_store::reference;
		using typename base_store::difference_type;
		using typename base_store::size_type;

		class iterator : public base_store::iterator
		{
			size_type m_offset = 0; // Relative to total of Stores
			const RecordStore *m_context = nullptr;
		public:

			// Construction
			iterator() = default;

			iterator(const RecordStore *context, size_type offset = 0)
				: m_offset(offset)
				, m_context(context)
			{
			}

			iterator(const iterator &it) = default;
			iterator& operator=(const iterator &it) = default;

			// Equality (!= auto-generated)
			bool operator==(const iterator &it) const
			{
				return (m_offset == it.m_offset && m_context == it.m_context);
			}

			// Three-way comparison (<, >, <=, >= auto-generated)
			std::strong_ordering operator<=>(const iterator &it) const
			{
				if (m_context != it.m_context)
					throw std::invalid_argument("Iterators incompatible");
				return m_offset <=> it.m_offset;
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
				if (m_context->size() <= 0 || m_offset == 0)
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
			iterator& operator+=(difference_type n)
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (n < 0) return *this -= static_cast<size_type>(-n);
				auto off = static_cast<size_type>(n);
				if (m_offset + off > m_context->size())
					throw std::length_error("Iterator + offset out of range");
				m_offset += off;
				return *this;
			}
			// Addition
			iterator operator+(difference_type n) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				tmp += n;
				return tmp;
			}
			// Compound subtraction assignment
			iterator& operator-=(difference_type n)
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (n < 0) return *this += static_cast<size_type>(-n);
				auto off = static_cast<size_type>(n);
				if (m_offset < off)
					throw std::length_error("Iterator - offset out of range");
				m_offset -= off;
				return *this;
			}
			// Subtraction
			iterator operator-(difference_type n) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				tmp -= n;
				return tmp;
			}
			difference_type operator-(iterator it) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				if (m_context != it.m_context)
					throw std::invalid_argument("Iterators incompatible");
				return difference_type(m_offset - it.m_offset);
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
			reference operator[](difference_type n) const
			{
				if (!m_context)
					throw std::invalid_argument("Invalid context");
				iterator tmp = *this;
				tmp += n;
				return *tmp;
			}

			// n + it (required for random_access_iterator)
			friend iterator operator+(difference_type n, const iterator& it)
			{
				return it + n;
			}
		};

		using const_iterator = iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		RecordStore() = delete;

		RecordStore(stream_type &stream, size_type recordSize, size_type storeSize, std::streamoff headerOffset)
			: store_type(stream)
			, m_recordSize(recordSize)
			, m_storeSize(storeSize)
			, m_headerOffset(headerOffset)
			, m_value(recordSize)
		{
		}

		iterator begin() const
		{
			return iterator(this);
		}
		const_iterator cbegin() const
		{
			return begin();
		}
		iterator end() const
		{
			return iterator(this, size());
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

		// Overrides
		virtual size_type size() const
		{
			return m_storeSize;
		}

	protected:
		virtual reference getR(size_type off) const
		{
			load(off);
			return m_value;
		}

		virtual pointer getP(size_type off) const
		{
			load(off);
			return &m_value;
		}

		virtual void load(size_type off) const = 0;

		size_type m_recordSize;
		size_type m_storeSize;
		std::streamoff m_headerOffset;
		mutable value_type m_value;
	};

}
