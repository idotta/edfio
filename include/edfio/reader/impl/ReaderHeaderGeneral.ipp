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
#include "../../header/HeaderGeneral.hpp"

#include <stdexcept>
#include <fstream>

namespace edfio
{

	HeaderGeneralFields ReaderHeaderGeneral::operator ()(Stream &stream)
	{
		HeaderGeneralFields hdr;
		if (!stream || !stream.is_open())
			throw std::invalid_argument(detail::GetError(FileErrc::FileNotOpened));
		
		stream.clear();
		stream.seekg(0, std::ios::beg);

		try
		{
			stream >> hdr.m_version;
			stream >> hdr.m_patient;
			stream >> hdr.m_recording;
			stream >> hdr.m_startDate;
			stream >> hdr.m_startTime;
			stream >> hdr.m_headerSize;
			stream >> hdr.m_reserved;
			stream >> hdr.m_datarecordsFile;
			stream >> hdr.m_datarecordDuration;
			stream >> hdr.m_totalSignals;
		}
		catch (std::exception e)
		{
			throw std::invalid_argument(detail::GetError(FileErrc::FileReadError));
		}
		return std::move(hdr);
	}

}
