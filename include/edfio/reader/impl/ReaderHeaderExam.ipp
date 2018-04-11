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
#include "../../header/HeaderExam.hpp"
#include "../ReaderHeaderGeneral.hpp"
#include "../ReaderHeaderSignal.hpp"
#include "../../processor/ProcessorHeaderGeneralFields.hpp"
#include "../../processor/ProcessorHeaderSignalFields.hpp"
#include "../../processor/ProcessorHeaderExam.hpp"

#include <stdexcept>
#include <fstream>

namespace edfio
{

	inline HeaderExam ReaderHeaderExam::operator ()(Stream &stream)
	{
		// Read general fields
		ReaderHeaderGeneral readerGeneral;
		auto generalFields = readerGeneral(stream);
		// Process general fields
		ProcessorHeaderGeneralFields procGeneralFields;
		auto general = std::move(procGeneralFields(std::move(generalFields)));

		// Read signal fields
		ReaderHeaderSignal readerSignals(general.m_totalSignals);
		auto signalFields = readerSignals(stream);
		// Process signal fields
		ProcessorHeaderSignalFields procSignalFields(general.m_version, general.m_datarecordDuration);
		auto signals = std::move(procSignalFields(std::move(signalFields)));

		// Process header exam
		ProcessorHeaderExam procHeader;
		auto header = std::move(procHeader(std::move(general), std::move(signals)));

		// File size
		{
			// get current position
			auto position = stream.tellg();
			// get length of file
			stream.seekg(0, stream.end);
			long long length = stream.tellg();
			// send back to previous position
			stream.seekg(position, stream.beg);

			if (length != (header.m_general.m_detail.m_recordSize * header.m_general.m_datarecordsFile + header.m_general.m_headerSize))
			{
				throw std::invalid_argument(detail::GetError(FileErrc::FileContainsFormatErrors));
			}
		}

		return std::move(header);
	}

}
