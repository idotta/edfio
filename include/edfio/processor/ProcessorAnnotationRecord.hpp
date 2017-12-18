//
// Copyright(c) 2017-present Iuri Dotta (dotta dot iuri at gmail dot com)
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.
//
// Official repository: https://github.com/idotta/edfio
//

#pragma once

#include "../Defs.hpp"
#include "ProcessorBase.hpp"
#include "../core/Record.hpp"
#include "../core/Annotation.hpp"

#include <string>

namespace edfio
{

	namespace detail
	{
		//    Each TAL starts with a time stamp Onset21Duration20

		static const char DURATION_DIV = 21;
		static const char ANNOTATION_DIV = 20;
		static const char ANNOTATION_END = 0;
	}

	struct ProcessorAnnotationRecord : ProcessorBase<RecordField, std::vector<Annotation>>
	{
		TypeO operator ()(TypeI in)
		{
			TypeO ou;
			auto& record = in();

			// Boundaries
			auto itFirst = record.begin();
			auto itLast = record.end() - 1;

			while (itLast != itFirst)
			{
				if (*itLast != detail::ANNOTATION_END)
					break;
				itLast--;
			}

			bool timestamp = true;
			for (auto it = itFirst; it != itLast; it++)
			{

				if ((*it == detail::ANNOTATION_END || it == itLast) && it != itFirst)
				{

					// TAL MUST start with '+' or '-'
					if (*itFirst != '+' && *itFirst != '-')
					{
						throw std::invalid_argument(detail::GetError(FileErrc::FileContainsInvalidAnnotations));
					}

					long long start = 0;
					long long duration = 0;
					bool onset = true;
					for (auto itTal = itFirst; itTal != it; itTal++)
					{

						if (std::distance(itFirst, itTal) > 0)
						{
							// First field is for TAL onset
							if (onset)
							{
								if (*itTal == detail::DURATION_DIV || *itTal == detail::ANNOTATION_DIV)
								{
									std::string tmp(itFirst, itTal);
									try
									{
										start = static_cast<long long>(std::stod(tmp) * TIME_DIMENSION);
									}
									catch (...)
									{
										throw std::invalid_argument(detail::GetError(FileErrc::FileContainsInvalidAnnotations));
									}
									onset = false;

									if (timestamp && *itTal == detail::ANNOTATION_DIV && *(itTal + 1) == detail::ANNOTATION_DIV)
									{
										Annotation annot;
										annot.m_start = start;
										annot.m_annotation = "Timestamp";
										ou.emplace_back(std::move(annot));
										timestamp = false;
									}
									
									itFirst = itTal + 1;
								}
							}
							else
							{
								if (*itTal == detail::DURATION_DIV)
								{
									std::string tmp(itFirst, itTal);
									try
									{
										duration = static_cast<long long>(std::stod(tmp) * TIME_DIMENSION);
									}
									catch (...)
									{
										throw std::invalid_argument(detail::GetError(FileErrc::FileContainsInvalidAnnotations));
									}
									itFirst = itTal + 1;
								}
								else if (*itTal == detail::ANNOTATION_DIV)
								{
									std::string tmp(itFirst, itTal);

									Annotation annot;
									annot.m_start = start;
									annot.m_duration = duration;
									annot.m_annotation = tmp;
									ou.emplace_back(std::move(annot));
									itFirst = itTal + 1;
								}
							}
						}
					}

					// itFirst equals next TAL beginning
					if (it + 1 != itLast)
						itFirst = it + 1;
				}
			}


			return ou;
		}
	};

}
