//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../../Utils.hpp"
#include "../../core/Record.hpp"
#include "../../core/Annotation.hpp"

#include <vector>

namespace edfio
{

	inline std::vector<Annotation> ProcessorTalRecord::operator()(std::vector<char> record, long long datarecord)
	{
		std::vector<Annotation> out;

		// Boundaries
		auto first = record.begin();
		auto last = record.end();

		// TAL MUST start with '+' or '-'
		if (*first != '+' && *first != '-')
		{
			throw std::invalid_argument(detail::GetError(FileErrc::FileContainsInvalidAnnotations));
		}

		double start = 0;
		double duration = 0;
		bool onset = true;
		for (auto it = first; it != last; it++)
		{
			if (std::distance(first, it) > 0)
			{
				// First field is for TAL onset
				if (onset)
				{
					if (*it == detail::DURATION_DIV || *it == detail::ANNOTATION_DIV)
					{
						std::string tmp(first, it);
						try
						{
							start = std::stod(tmp);
						}
						catch (...)
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsInvalidAnnotations));
						}
						onset = false;

						first = it;
					}
				}
				else if (*it == detail::ANNOTATION_DIV)
				{
					if (*first == detail::DURATION_DIV)
					{
						std::string tmp(first + 1, it);
						try
						{
							duration = std::stod(tmp);
						}
						catch (...)
						{
							throw std::invalid_argument(detail::GetError(FileErrc::FileContainsInvalidAnnotations));
						}
						first = it;
					}
					else if (*first == detail::ANNOTATION_DIV)
					{
						std::string tmp(first + 1, it);

						// Check if this TAL is in fact just a timestamp, we don't want this
						if (!tmp.empty())
						{
							Annotation annot;
							annot.m_start = start;
							annot.m_duration = duration;
							annot.m_annotation = tmp;
							annot.m_dararecord = datarecord;
							out.emplace_back(std::move(annot));
						}
						first = it;
					}
				}
			}

		}
		return std::move(out);
	}

}
